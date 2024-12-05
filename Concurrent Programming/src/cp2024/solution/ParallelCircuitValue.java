package cp2024.solution;

import cp2024.circuit.CircuitValue;

public class ParallelCircuitValue implements CircuitValue {

    private boolean stopFlag;
    private boolean calculated = false;
    private boolean result;

    public ParallelCircuitValue(boolean broken) {
        this.stopFlag = broken;
    }

    @Override
    public synchronized boolean getValue() throws InterruptedException {
        if (calculated) {
            return result;
        }
        while (!calculated && !stopFlag) {
            wait();
        }
        if (stopFlag) {
            throw new InterruptedException();
        }
        
        return result;
    }

    public synchronized void stop() {
        stopFlag = true;
        notifyAll();
    }

    public synchronized void calculated(boolean result) {
        this.result = result;
        calculated = true;
        notifyAll();
    }

}
