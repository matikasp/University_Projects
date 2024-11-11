#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define eps 1e-10

typedef enum {
    LEFT, RIGHT, CENTER
} PointPosition;

typedef enum {
    RECTANGLE, CIRCLE, COMPOSITE
} SheetType;

typedef struct {
    double x;
    double y;
} Point;

// Line represented in general form Ax + By + C = 0
// Where 
// A = P1.y - P2.y
// B = P2.x - P1.x
// C = P1.y * (P1.x - P2.x) + P1.x * (P2.y - P1.y)
// then the properties hold (easy to prove):
// if Axp + Byp + C > 0 then P is on the left
// if Axp + Byp + C < 0 then P is on the right
// visualization: https://www.desmos.com/calculator/kev7gyyzgd
typedef struct {
    double A;
    double B;
    double C;
} Line;

typedef struct Sheet {
    SheetType type;
    union {
        struct { // Rectangle parameters
            Point P1;
            Point P2;
        } RectangleParams;
        struct { // Circle parameters
            Point center;
            double r;
        } CircleParams;
        struct { // Composite parameters
            Line compositeLine;
            struct Sheet* sheet;
        } CompositeParams;
    };
} Sheet;

// Helper functions
bool eq(double a, double b);  // Equal (==)
bool geq(double a, double b); // Greater or equal (>=)
bool leq(double a, double b); // Less or equal (<=)
PointPosition side(Point P, Line line); // Returns LEFT, RIGHT, or CENTER
Point reflect_point(Point P, Line line); // Symmetry with respect to the line
bool is_inside(Point P, Sheet sheet); // For sheets of type CIRCLE or RECTANGLE
                                      // returns whether the point is inside or outside

// Constructors
Sheet circle(Point center, double r);
Sheet rectangle(Point P1, Point P2);
Sheet composite(Point P1, Point P2, Sheet *sheet);
Line general_form_of_line(Point P1, Point P2); // Returns the line in general form

// Function that performs the task
int number_of_layers(Sheet sheet, Point P);

int main() {
    int n, q;
    scanf("%d%d", &n, &q);

    // Array for sheets
    Sheet *sheet_array = (Sheet *)malloc((unsigned)n * sizeof(Sheet));

    // Input handling
    for (int i = 0; i < n; i++) {
        Sheet sheet;
        char type;
        scanf(" %c", &type);
        switch (type) {
            case 'R': {
                Point P1, P2;
                scanf("%lf%lf%lf%lf", &P1.x, &P1.y, &P2.x, &P2.y);
                sheet = rectangle(P1, P2);
                break;
            }
            case 'C': {
                Point center;
                double r;
                scanf("%lf%lf%lf", &center.x, &center.y, &r);
                sheet = circle(center, r);
                break;
            }
            case 'F': {
                int k;
                Point P1, P2;
                scanf("%d%lf%lf%lf%lf", &k, &P1.x, &P1.y, &P2.x, &P2.y);
                sheet = composite(P1, P2, &sheet_array[k-1]);
                break;
            }
            default: // Handling invalid input
                assert(0 && "Invalid input");
                break;
        }
        sheet_array[i] = sheet;
    }

    for (int i = 0; i < q; i++) {
        int k;
        Point P;
        scanf("%d%lf%lf", &k, &P.x, &P.y);
        printf("%d\n", number_of_layers(sheet_array[k-1], P));
    }

    free(sheet_array);

    return 0;
}

bool eq(double a, double b) {
    return fabs(a - b) < eps;
}

bool leq(double a, double b) {
    return a < b || eq(a, b);
}

bool geq(double a, double b) {
    return a > b || eq(a, b);
}

PointPosition side(const Point P, const Line line) {
    if (eq(line.A * P.x + line.B * P.y + line.C, 0))
        return CENTER;

    // for the line in this form always if Axp + Byp + C > 0 then P is on the left
    if (line.A * P.x + line.B * P.y + line.C > 0)
        return LEFT;

    // for the line in this form always if Axp + Byp + C < 0 then P is on the right
    return RIGHT;
}

Point reflect_point(Point P, const Line line) {
    if (eq(line.B, 0)) {
        P.x -= 2 * (P.x + line.C / line.A);
    }
    else if (eq(line.A, 0)) {
        P.y -= 2 * (P.y + line.C / line.B);
    }
    else {
        double m = -line.A / line.B;             // variables to represent the slope-intercept form
        double p = -line.C / line.B;             // of the line (y = mx + p) and the line
        double q = P.y - P.x * line.B / line.A;  // perpendicular (y = (-1/m)x + q), passing through point P

        Point intersection; // intersection of these lines
        intersection.x = m * (q - p) / (m * m + 1);
        intersection.y = (m * m * q + p) / (m * m + 1);

        // shift the coordinate system so that the intersection is at the origin
        P.x -= intersection.x;
        P.y -= intersection.y;

        // symmetry with respect to the origin
        // since the system has been shifted, this is equivalent to symmetry with respect to the line
        P.x = -P.x;
        P.y = -P.y;

        // shift the system back
        P.x += intersection.x;
        P.y += intersection.y;
    }
    return P;
}

Sheet circle(const Point center, const double r) {
    Sheet sheet;
    sheet.type = CIRCLE;
    sheet.CircleParams.center = center;
    sheet.CircleParams.r = r;
    return sheet;
}

Sheet rectangle(const Point P1, const Point P2) {
    Sheet sheet;
    sheet.type = RECTANGLE;
    sheet.RectangleParams.P1 = P1;
    sheet.RectangleParams.P2 = P2;
    return sheet;
}

Sheet composite(const Point P1, const Point P2, Sheet *sheet) {
    Line line = general_form_of_line(P1, P2);
    Sheet composite_sheet;
    composite_sheet.type = COMPOSITE;
    composite_sheet.CompositeParams.compositeLine = line;
    composite_sheet.CompositeParams.sheet = sheet;
    return composite_sheet;
}

Line general_form_of_line(const Point P1, const Point P2) {
    Line line;
    line.A = P1.y - P2.y;
    line.B = P2.x - P1.x;
    line.C = P1.y * (P1.x - P2.x) + P1.x * (P2.y - P1.y);
    return line;
}

int number_of_layers(const Sheet sheet, const Point P) {
    if (sheet.type == RECTANGLE) {
        Point P1 = sheet.RectangleParams.P1;
        Point P2 = sheet.RectangleParams.P2;

        // returns 1 if P is inside the rectangle, 0 otherwise
        return geq(P.x, P1.x) && leq(P.x, P2.x) && geq(P.y, P1.y) && leq(P.y, P2.y);
    }

    if (sheet.type == CIRCLE) {
        Point center = sheet.CircleParams.center;
        double r = sheet.CircleParams.r;

        // returns 1 if P is inside the circle, 0 otherwise
        return leq(pow((P.x - center.x), 2) + pow((P.y - center.y), 2), r * r);
    }

    // if the sheet is a composite (type == COMPOSITE)
    Line line = sheet.CompositeParams.compositeLine;
    switch (side(P, line)) {
        case RIGHT:
            return 0;
        case CENTER:
            return number_of_layers(*sheet.CompositeParams.sheet, reflect_point(P, line));
        case LEFT:
            return number_of_layers(*sheet.CompositeParams.sheet, P) +
                   number_of_layers(*sheet.CompositeParams.sheet, reflect_point(P, line));
        default: // handling unexpected values
            assert(0 && "Unexpected value");
            break;
    }
}