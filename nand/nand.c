#include "nand.h"
#include "queue.h"

#include <stdlib.h>
#include <errno.h>


#define max(a, b) ((a) > (b) ? (a) : (b))

// typ - sygnal boolowski lub bramka nand
enum Type {
    BOOL,
    NAND
};

// wartosc logiczna bramki - wyznaczona, w trakcie lub jeszcze nie
// zeby w nand_evaluate dla kazdej z bramki wyznaczyc jej wartosc tylko raz
enum OutputState {
    EVALED,
    EVALING,
    NEVAL
};

// sygnal boolowski traktuje jako rodzaj nanda
// dlatego robie tutaj unie
struct nand {
    enum Type type;
    union {
        struct {
            nand_t **input;
            unsigned input_size;

            enum OutputState state;
            ssize_t critical_length;
            bool output;

            Queue* outputs;
        };
        bool* logic_val;
    };
};

// tworzenie nowej bramki
nand_t* nand_new(unsigned n) {
    // alokacja pamieci
    nand_t* new_nand = (nand_t*)malloc(sizeof(nand_t));
    if (new_nand == NULL) {
        errno = ENOMEM;
        return NULL;
    }
    new_nand->input = (nand_t**)malloc(n * sizeof(nand_t*));
    if (new_nand->input == NULL) {
        errno = ENOMEM;
        free(new_nand);
        return NULL;
    }
    Queue* queuePtr = newQueue();
    if (queuePtr == NULL) {
        errno = ENOMEM;
        free(new_nand->input);
        free(new_nand);
        return NULL;
    }

    // jesli alokacje sie powiodly to zapelnienie tablicy input nullami
    // oraz ustawienie pozostalych zmiennych
    for (unsigned idx = 0; idx < n; ++idx) {
        new_nand->input[idx] = NULL;
    }
    new_nand->outputs = queuePtr;
    new_nand->type = NAND;
    new_nand->state = NEVAL;
    new_nand->input_size = n;
    new_nand->output = false;
    new_nand->critical_length = 0;
    return new_nand;
}

// usuwanie bramki
void nand_delete(nand_t *g) {
    if (g == NULL) {
        return;
    }
    // jesli funkcja dostala na wejsciu wartosc boolowska to
    // traktuje ja jako bledny parametr i nic nie robi
    if (g->type == BOOL) {
        return;
    }
    // usuwamy wszystkie wystapienia node'ow w outputs bramek wejsciowych
    for (unsigned idx = 0; idx < g->input_size; ++idx) {
        if (g -> input[idx] == NULL) {
            continue;
        }
        else if (g->input[idx]->type == NAND) {
            deleteNode(g->input[idx]->outputs, g);
        }
        else { // jesli g->input[idx]->type == BOOL
            // wtedy zwalniamy wartosc boolowska, bo nie jest juz potrzebna
            free(g->input[idx]);
        }
    }
    // usuwamy z tablic input bramek z outputs wszystkie wystapienia g
    for (ssize_t idx = 0; idx < g->outputs->size; ++idx) {
        nand_t* tmp = iterQueue(g->outputs, idx);
        for (unsigned j = 0; j < tmp->input_size; ++j) {
            if (tmp->input[j] == g) {
                tmp->input[j] = NULL;
            }
        }
    }

    // zwolnienie pamieci po bramce, kolejce outputs i tablicy input
    free(g->input);
    freeQueue(g->outputs);
    free(g);
}

// podlaczenie wyjscia jednej bramki do k-tego wejscia drugiej
int nand_connect_nand(nand_t *g_out, nand_t *g_in, unsigned k) {

    // obsluga nieprawidlowych wartosci
    if (g_in == NULL || g_out == NULL) {
        errno = EINVAL;
        return -1;
    }
    if (k >= g_in->input_size) {
        errno = EINVAL;
        return -1;
    }

    // obsluga niepowodzenia w alokacji pamieci
    if (push(g_out->outputs, g_in) == -1) {
        errno = ENOMEM;
        return -1;
    }

    // czyscimy wejscie bramki oraz usuwamy jedno wystapienie
    // bramki w outputs bramki, ktora byla na jej k-tym wejsciu
    if (g_in->input[k] != NULL && g_in->input[k]->type == NAND) {
        deleteNode(g_in->input[k]->outputs, g_in);
    }
    if(g_in->input[k]!=NULL&&g_in->input[k]->type==BOOL)free(g_in->input[k]);

    g_in->input[k] = g_out;

    return 0;
}

// tworzy wartosc boolowska
nand_t* create_bool(bool* logic_val) {
    // alokacja pamieci
    nand_t* new_bool = (nand_t*)malloc(sizeof(nand_t));
    if (new_bool == NULL) {
        return NULL;
    }

    // ustawienie zmiennych na porzadane wartosci
    new_bool->type = BOOL;
    new_bool->logic_val = logic_val;
    return new_bool;
}

// podlacza sygnal boolowski do k-tego wejscia bramki
int nand_connect_signal(bool const *s, nand_t *g, unsigned k) {
    if (s == NULL || g == NULL) {
        errno = EINVAL;
        return -1;
    }
    if (k >= g->input_size) {
        errno = EINVAL;
        return -1;
    }
    nand_t* tmp = create_bool((bool*)s);
    if (tmp == NULL) {
        errno = ENOMEM;
        return -1;
    }
    if (g->input[k] != NULL && g->input[k]->type == NAND) {
        deleteNode(g->input[k]->outputs, g);
    }
    if(g->input[k]!=NULL&&g->input[k]->type==BOOL)free(g->input[k]);
    g->input[k] = tmp;
    return 0;
}

// po nand_evaluate chcemy zamienic w kazdej bramce ktora odwiedzilismy stan
// z powrotem na NEVAL
void clear_eval(nand_t *g) {
    // jesli trafilismy na nulla, boola albo neval to znaczy ze
    // albo doszlismy do "poczatku" grafu bramek
    // albo przeszlismy po calym cyklu jesli taki wystepowal
    if (g == NULL || g->type == BOOL || g->state == NEVAL) {
        return;
    }

    // ustawiamy g->state na NEVAL i czyscimy rekurencyjnie bramki
    g->state = NEVAL;

    for (unsigned idx = 0; idx < g->input_size; ++idx) {
        clear_eval(g->input[idx]);
    }
}

// struct do wygodnego zwracania stanu bramki (output + dlugosc krytyczna)
struct nand_state {
    bool output;
    ssize_t critical_length;
};

// rekurencyjnie wyznacza dlugosc sciezki krytycznej danej bramki
// (rekurencja ze spamietywaniem)
struct nand_state nand_evaluate_rec(nand_t *g) {
    struct nand_state result;

    // przypadki podstawowe
    // BOOL - doszlismy do "poczatku grafu"
    // g->input_size == 0 - trafilismy na bramke bez wejsc
    // EVALED - do wierzcholka, ktory zostal obliczony wczesniej
    // EVALING - trafilismy na cykl

    if (g->type == BOOL) {
        result.output = *(g->logic_val);
        result.critical_length = 1;
        return result;
    }
    if (g->input_size == 0) {
        g->state = EVALED;
        result.output = false;
        result.critical_length = 1;
        return result;
    }
    if (g->state == EVALED) {
        result.output = g->output;
        result.critical_length = g->critical_length+1;
        return result;
    }
    if (g->state == EVALING) {
        errno = ECANCELED;
        result.critical_length = -1;
        return result;
    }

    // rekurencyjnie wyznaczamy dlugosc sciezki krytycznej i wartosci logiczne
    result.critical_length = 0;
    result.output = false;
    g->state = EVALING;
    for (unsigned idx = 0; idx < g->input_size; ++idx) {
        // jesli trafimy na NULL na wejsciu danej bramki to nie mozemy policzyc
        if (g->input[idx] == NULL) {
            errno = ECANCELED;
            result.critical_length = -1;
            return result;
        }
        struct nand_state eval_curr = nand_evaluate_rec(g->input[idx]);
        // jesli dlugosc sciezki krytycznej to -1 to znaczy ze
        // przerwano obliczanie dlugosci sciezki krytycznej, wiec konczymy
        if (eval_curr.critical_length == -1) {
            result.critical_length = -1;
            return result;
        }
        // w przeciwnym przypadku dlugosc sciezki krytycznej ustawiamy
        // na max z dlugosci sciezek krytycznej na wyjsciach bramek z wejscia
        result.critical_length = max(result.critical_length,
                                     eval_curr.critical_length);
        // obliczamy wartosc logiczna na wyjsciu
        // (prznajmniej 1 false oznacza, ze na wyjsciu mamy true)
        if (!eval_curr.output) {
            result.output = true;
        }
    }

    // spamietujemy wartosci dlugosci sciezki krytycznej dla danej bramki
    g->critical_length = result.critical_length;
    g->output = result.output;
    g->state = EVALED;

    // zwracamy dlugosc sciezki krytycznej na wyjsciu bramki (tzn. o 1 wiecej)
    ++result.critical_length;
    return result;
}

// obliczamy dlugosc sciezki krytycznej oraz wartosci logiczne bramek na output
ssize_t nand_evaluate(nand_t **g, bool *s, size_t m) {
    // sprawdzamy poprawnosc danych
    if (m <= 0 || g == NULL || s == NULL) {
        errno = EINVAL;
        return -1;
    }
    for (unsigned idx = 0; idx < m; ++idx) {
        if (g[idx] == NULL) {
            errno = EINVAL;
            return -1;
        }
    }

    // wywolujemy rekurencyjne obliczenie dlugosci sciezki krytycznej i output
    // dla kazdej z bramek z tablicy g
    int max_ = 0;
    for (unsigned idx = 0; idx < m; ++idx) {
        struct nand_state eval_curr = nand_evaluate_rec(g[idx]);
        if (eval_curr.critical_length == -1) {
            // czyscimy stany bramek i zwracamy -1
            for (unsigned idx = 0; idx < m; ++idx) {
                clear_eval(g[idx]);
            }
            return -1;
        }
        s[idx] = eval_curr.output;
        max_ = max(max_, eval_curr.critical_length - 1);
    }

    // czyscimy stany bramek
    for (unsigned idx = 0; idx < m; ++idx) {
        clear_eval(g[idx]);
    }

    // zwracamy maksymalna dlugosc z sciezek krytycznych
    return max_;
}

// liczba bramek podlaczonych do output
ssize_t nand_fan_out(nand_t const *g) {
    // sprawdzamy poprawnosc danych
    if (g == NULL) {
        errno = EINVAL;
        return -1;
    }

    // zwracamy dlugosc outputs
    return g->outputs->size;
}

// zwracamy wskaznik na bramke podlaczona do k-tego wejscia
void* nand_input(nand_t const *g, unsigned k) {
    // sprawdzamy poprawnosc danych
    if (g == NULL || k >= g->input_size) {
        errno = EINVAL;
        return NULL;
    }
    if (g->input[k] == NULL) {
        errno = 0;
        return NULL;
    }
    // jesli bramke na k-tym wejsciu jest boolem, to zwracamy wskaznik na boola
    // na ktory wskazuje wskaznik w struct'cie
    if (g->input[k]->type == BOOL) {
        return g->input[k]->logic_val;
    }

    // w przeciwnym przypadku zwracamy wskaznik na bramke
    return g->input[k];
}

// zwracamy jedna z bramek podlaczonych do wyjscia bramek w taki sposob, ze
// jesli wywolamy funkcje dla wszystkich k \in [0, nand_fan_out(g)], to kazda
// z bramek podlaczonych do outputa pojawi sie dokladnie raz
nand_t* nand_output(nand_t const *g, ssize_t k) {
    return (nand_t*)iterQueue(g->outputs, k);
}
