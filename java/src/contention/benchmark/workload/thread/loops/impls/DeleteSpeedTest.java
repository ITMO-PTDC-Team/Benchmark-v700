package contention.benchmark.workload.thread.loops.impls;

import java.lang.reflect.Method;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.List;
import java.util.Queue;
import java.util.concurrent.atomic.AtomicInteger;

import contention.abstractions.CompositionalMap;
import contention.abstractions.DataStructure;
import contention.benchmark.tools.Range;
import contention.benchmark.workload.thread.loops.abstractions.ThreadLoop;

public class DeleteSpeedTest extends ThreadLoop {

    public DeleteSpeedTest(int myThreadNum,
                           DataStructure<Integer> dataStructure,
//                           CompositionalMap<Integer, Integer> bench,
                           Method[] methods) {
        super(myThreadNum, dataStructure, methods, null);

    }

    private List<Integer> vertices;
    private int lastLayer;


    @Override
    public void run() {
        long startTime = System.currentTimeMillis();
        long startNanoTime = System.nanoTime();

        for (int i = lastLayer; i < vertices.size(); i++) {
            executeRemove(vertices.get(i));
        }

//        while (!bench.isEmpty());
        // TODO The isEmpty() method includes logically removed nodes, when the size() does not
        while (executeSize() > 0);

        long endTime = System.currentTimeMillis();
        long endNanoTime = System.nanoTime();

        long elapsedTime = endTime - startTime;
        long elapsedNanoTime = endNanoTime - startNanoTime;

        System.out.println("\n===========================================================\n"
                + "Delete time:         \t" + elapsedTime + "\n"
                + "Delete nano time:    \t" + elapsedNanoTime
                + "\n===========================================================\n");

        // System.out.println(numAdd + " " + numRemove + " " + failures);
        this.stats.getCount = CompositionalMap.counts.get().getCount;
        this.stats.nodesTraversed = CompositionalMap.counts.get().nodesTraversed;
        this.stats.realNodesDeleted = CompositionalMap.counts.get().realNodesDeleted;
        this.stats.insertNodesTraversed = CompositionalMap.counts.get().insertNodesTraversed;
        this.stats.deleteNodesTraversed = CompositionalMap.counts.get().deleteNodesTraversed;
        this.stats.structMods = CompositionalMap.counts.get().structMods;
        this.stats.foundCnt = CompositionalMap.counts.get().foundCnt;
        this.stats.foundTreeTraversed = CompositionalMap.counts.get().foundTreeTraversed;
        this.stats.foundLogicalTraversed = CompositionalMap.counts.get().foundLogicalTraversed;
        this.stats.notFoundCnt = CompositionalMap.counts.get().notFoundCnt;
        this.stats.notFoundTreeTraversed = CompositionalMap.counts.get().notFoundTreeTraversed;
        this.stats.notFoundLogicalTraversed = CompositionalMap.counts.get().notFoundLogicalTraversed;
        this.stats.failedLockAcquire = CompositionalMap.counts.get().failedLockAcquire;
        System.out.println("Thread #" + threadId + " finished.");
    }

    @Override
    public void step() {

    }

    public void prefill(AtomicInteger prefillSize) {
        int size = prefillSize.get();
        vertices = new ArrayList<>(size);
        Queue<Range> vertQueue = new ArrayDeque<>();

        vertQueue.add(new Range(1, size));

        while (!vertQueue.isEmpty()) {
            Range next = vertQueue.remove();

            if (next.left > next.right) {
                continue;
            }

            int nextVert = (next.left + next.right) / 2;

            vertices.add(nextVert);
            executeInsert(nextVert);
//            bench.putIfAbsent(nextVert, nextVert);

            if (next.left == next.right) {
                continue;
            }

            vertQueue.add(new Range(next.left, nextVert - 1));
            vertQueue.add(new Range(nextVert + 1, next.right));
        }

        for (int i = 0, deg = 1; i + deg < size; deg <<= 1) {
            for (int j = 0; j < deg; j++) {
//                bench.remove(vertices.get(i + j));
                executeRemove(vertices.get(i + j));
            }
            i += deg;
            lastLayer = i;
        }
    }
}
