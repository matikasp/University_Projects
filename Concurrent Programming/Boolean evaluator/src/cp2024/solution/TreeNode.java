package cp2024.solution;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.Future;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ArrayBlockingQueue;

import cp2024.circuit.*;

/**
 * The TreeNode class represents a node in a boolean circuit tree.
 * Each TreeNode is associated with a CircuitNode and can have child TreeNodes.
 * The class implements the Callable interface, allowing it to be executed by an ExecutorService.
 * 
 * The call() method is the main execution method, which processes the node's logic and communicates
 * results to the parent node.
 */

class TreeNode implements Callable<Void> {
    // Index of this node in the parent's children list
    private final int index;

    private NodeType type;
    private final CircuitNode circuitNode;
    private final TreeNode parent;
    private List<TreeNode> children;

    // Flag to indicate if the task ended or was cancelled in this node
    private AtomicBoolean stopFlag;

    private boolean isInitialized;

    // Queue to store the result of the node's computation (used like a pipe)
    private BlockingQueue<Boolean> result;

    // Queue to store the indices of completed child computations
    private BlockingQueue<Integer> calculationsDone;

    // Number of child nodes (it is a maximum number of results needed)
    private int maxResultNeeded;

    // Counters for the number of true and false results received from children
    private int gotTrue, gotFalse;

    // for LT and GT nodes
    private int threshold;

    // for IF nodes
    private int [] results;

    private final ExecutorService executor;
    private Future<Void> futureTask;

    
    /**
     * Constructs a TreeNode object.
     *
     * @param circuitNode the CircuitNode associated with this TreeNode
     * @param parent the parent TreeNode
     * @param executor the ExecutorService used for concurrent execution
     * @param index the index of this TreeNode in a parent's children list
     */
    public TreeNode(CircuitNode circuitNode, TreeNode parent, ExecutorService executor, int index) {
        this.index = index;
        this.circuitNode = circuitNode;
        this.type = circuitNode.getType();
        this.parent = parent;
        this.stopFlag = new AtomicBoolean(false);
        this.isInitialized = false;
        this.calculationsDone = new LinkedBlockingQueue<>();
        this.executor = executor;
        this.result = new ArrayBlockingQueue<>(1);
        this.gotTrue = 0;
        this.gotFalse = 0;
        
        if (type == NodeType.GT || type == NodeType.LT) {
            this.threshold = ((ThresholdNode)circuitNode).getThreshold();
        }
        if (type == NodeType.IF) {
            this.results = new int[] {-1, -1, -1};
        }

        futureTask = executor.submit(this);
    }

    /**
     * Method to cancel the the subtree if the parent node is done calculating.
     * Cancels the current node and all its descendants in the subtree.
     * 
     * The method first cancels the current node by calling {@link #cancel()}.
     * If the node is initialized and has children, it recursively cancels
     * all non-stopped child nodes by calling their {@code cancelSubtree()} method.
     */
    public void cancelSubtree() {
        this.cancel();
        if (isInitialized() && children != null) {
            for (TreeNode child : children) {
                if (!child.isStopped()) {
                    child.cancelSubtree();
                }
            }
        }
    }

    public synchronized void cancel() {
        if (stopFlag.get()) {
            return;
        }
        stopFlag.set(true);
        if (futureTask != null) {
            futureTask.cancel(true);
        }
    }

    public boolean isStopped() {
        return stopFlag.get();
    }

    public boolean isInitialized() {
        return isInitialized;
    }

    public Boolean getResult() throws InterruptedException {
        return result.take();
    }

    public Boolean isResultReady() {
        return !result.isEmpty();
    }

    /**
     * Puts the index of the current node in the parent's calculationsDone queue.
     * This method is called when the current node has completed its computation.
     * If parent is null, the method does nothing, because the node is a root of the tree.
     */
    private void informParent() throws InterruptedException {
        if (parent != null) {
            parent.putIndex(index);
        }
    }
    
    /**
     * Puts the index of a child node in the calculationsDone queue.
     * This method is called by a child node when it has completed its computation.
     * @param index the index of the child node
     */
    public void putIndex(int index) throws InterruptedException {
        calculationsDone.put(index);
    }


    /**
     * Executes the logic for the TreeNode. This method is called when the TreeNode is run as a task.
     * It performs initialization if necessary, processes the node based on its type, and handles
     * the results of child nodes. The method also manages stopping conditions and interruption handling.
     *
     * @return null
     */
    @Override
    public Void call() {
        try {
            while (!stopFlag.get() && !Thread.currentThread().isInterrupted()) {
                if (!isInitialized) {
                    initialize();
                }
                if (stopFlag.get()) {
                    break;
                }
                if (type == NodeType.LEAF) {
                    // Process leaf node and put its value in the result queue
                    result.put(((LeafNode)circuitNode).getValue());
                    informParent();
                    return null;
                } else {
                    // Wait for a child computation to complete
                    int idx = calculationsDone.take();
                    boolean res = children.get(idx).getResult();
                    if (res) {
                        gotTrue++;
                    } else {
                        gotFalse++;
                    }
                    int ans = -1;
                    // Solve the node based on its type
                    switch (type) {
                        case NOT -> ans = solveNOT();
                        case LT -> ans = solveLT();
                        case GT -> ans = solveGT();
                        case OR -> ans = solveOR();
                        case AND -> ans = solveAND();
                        case IF -> ans = solveIF(idx, res);
                        default -> throw new RuntimeException("Illegal type " + type);
                    }
                    // If a definitive result is obtained, put it in the result queue and stop
                    if (ans == 1) {
                        result.put(true);
                        informParent();
                        stopFlag.set(true);
                        break;
                    } else if (ans == 0) {
                        result.put(false);
                        informParent();
                        stopFlag.set(true);
                        break;
                    }
                }
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt(); // Preserve interrupt status
        } finally {
            this.stopFlag.set(true);
            // Cancel all child nodes if not already stopped
            for (TreeNode child : children) {
                if (!child.isStopped()) {
                    child.cancelSubtree();
                }
            }
            this.cancel();
        }
        return null;
    }

    /**
     * Initializes the TreeNode by setting up its children and evaluating its result if possible.
     * If the node is a leaf, it marks itself as initialized and returns immediately.
     * For LT (less than) and GT (greater than) node types, it tries to evaluate the result based on the threshold
     * and the number of arguments, and if a result can be determined, it sets the result, informs the parent,
     * and stops further processing.
     * For non-leaf nodes (for LT and GT if they were not evaluated), 
     * it creates child TreeNodes for each argument and adds them to the children list.
     * Finally, it marks the node as initialized.
     *
     * @throws InterruptedException if the thread executing this method is interrupted
     */
    private void initialize() throws InterruptedException {
        if (type == NodeType.LEAF) {
            isInitialized = true;
            return;
        }

        CircuitNode[] args = circuitNode.getArgs();
        maxResultNeeded = args.length;
        children = new ArrayList<>(args.length);
        
        if (type == NodeType.LT) {
            if (threshold == 0) {
                result.put(false);
                informParent();
                stopFlag.set(true);
                return;
            }
            if (threshold > maxResultNeeded) {
                result.put(true);
                informParent();
                stopFlag.set(true);
                return;
            }
        }
        if (type == NodeType.GT && threshold >= maxResultNeeded) {
            result.put(false);
            informParent();
            stopFlag.set(true);
            return;
        }

        for (int i = 0; i < args.length; i++) {
            children.add(new TreeNode(args[i], this, executor, i));
        }

        isInitialized = true;
    }


    /**
     * The following methods solve the node based on the results of the children.
     * If the result can be determined, the method returns the result. Otherwise, it returns -1.
     */

    private int solveNOT() throws InterruptedException {
        return gotTrue == 1 ? 0 : 1;
    }

    private int solveLT() throws InterruptedException {
        if (gotTrue >= threshold) {
            return 0;
        }
        if (gotFalse > maxResultNeeded - threshold) {
            return 1;
        }
        return -1;
    }
    private int solveGT() throws InterruptedException {
        if (gotTrue > threshold) {
            return 1;
        }
        if (gotFalse >= maxResultNeeded - threshold) {
            return 0;
        }
        return -1;
    }

    private int solveOR() throws InterruptedException {
        if (gotTrue > 0) {
            return 1;
        }
        if (gotFalse == maxResultNeeded) {
            return 0;
        }
        return -1;
    }

    private int solveAND() throws InterruptedException {
        if (gotFalse > 0) {
            return 0;
        }
        if (gotTrue == maxResultNeeded) {
            return 1;
        }
        return -1;
    }

    /*
     * For IF nodes, the method solves the node based on the results of the children.
     * If the result is determined, it returns the result. Otherwise, it returns -1.
     * @param idx the index of the child node that has completed its computation
     * @param res the result of the child node
     */
    private int solveIF(int idx, boolean res) throws InterruptedException {
        results[idx] = res ? 1 : 0;
        if (results[0] != -1) {
            if (results[0] == 1) {
                if (!children.get(2).isStopped()) {
                    children.get(2).cancelSubtree();
                }
                return results[1];
            } else {
                if (!children.get(1).isStopped()) {
                    children.get(1).cancelSubtree();
                }
                return results[2];
            }
        }
        if (results[1] == results[2]) {
            if (results[1] != -1 && !children.get(0).isStopped()) {
                children.get(0).cancelSubtree();
            }
            return results[1];
        }

        return -1;
    }
}
