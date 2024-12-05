package cp2024.solution;

import java.util.concurrent.ExecutorService;

import cp2024.circuit.CircuitNode;

public class Tree {
    private TreeNode root;

    public Tree(CircuitNode root, ExecutorService executor, ParallelCircuitValue value) {
        this.root = new TreeNode(root, null, executor, 0);
        executor.submit(() -> {
            try {
                value.calculated(this.root.getResult());
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });
    }
}
