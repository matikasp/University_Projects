package cp2024.solution;

import java.util.concurrent.ExecutorService;
import java.util.ArrayList;
import java.util.concurrent.Executors;

import cp2024.circuit.CircuitSolver;
import cp2024.circuit.CircuitValue;
import cp2024.circuit.Circuit;

public class ParallelCircuitSolver implements CircuitSolver {
    private ExecutorService executor;
    private boolean isExecutorInitialized = false;
    private boolean stopFlag = false;
    private ArrayList<ParallelCircuitValue> values = new ArrayList<>();

    @Override
    public CircuitValue solve(Circuit c) {
        if (!stopFlag) {
            if (!isExecutorInitialized) {
                isExecutorInitialized = true;
                executor = Executors.newCachedThreadPool(r -> {
                    Thread t = new Thread(r);
                    t.setDaemon(true);
                    return t;
                });
                ((java.util.concurrent.ThreadPoolExecutor) executor).setKeepAliveTime(1, java.util.concurrent.TimeUnit.SECONDS);
            }
            ParallelCircuitValue parallelCircuitValue = new ParallelCircuitValue(false);
            values.add(parallelCircuitValue);
            new Tree(c.getRoot(), executor, parallelCircuitValue);
            return parallelCircuitValue;
        } else {
            return new ParallelCircuitValue(true);
        }
    }

    @Override
    public void stop() {
        stopFlag = true;
        if (isExecutorInitialized) {
            executor.shutdownNow();
        }
        for (ParallelCircuitValue value : values) {
            value.stop();
        }
    }
}
