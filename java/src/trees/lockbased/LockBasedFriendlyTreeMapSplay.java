package trees.lockbased;

import java.util.AbstractMap;
import java.util.Comparator;
import java.util.Set;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ThreadLocalRandom;
import java.util.concurrent.locks.ReentrantLock;

import contention.abstractions.CompositionalMap;
import contention.abstractions.CompositionalMap.Vars;

/**
 * The contention-friendly tree implementation of map 
 * as described in:
 *
 * T. Crain, V. Gramoli and M. Ryanla. 
 * A Contention-Friendly Binary Search Tree. 
 * Euro-Par 2013.
 * 
 * @author Tyler Crain
 * 
 * @param <K>
 * @param <V>
 */

public class LockBasedFriendlyTreeMapSplay<K, V> extends AbstractMap<K, V> implements
		CompositionalMap<K, V> {

	static final boolean useFairLocks = false;
	static final boolean allocateOutside = true;
	// we encode directions as characters
	static final char Left = 'L';
	static final char Right = 'R';
	final V DELETED = (V) new Object();

	public enum RebalanceMode {
		Splay,
		None,
	}

	static final boolean STRUCT_MODS = true;

	private double splayProb = 1.0 / 8;
	private double threadNum = 8;
	private double k1 = 3.0;
	private double k2 = 0.5;
	private int maxDepth = 5;

	private double rotateProb(final long iterations, final long depth) {
		if (iterations == 0) {
			return splayProb;
		}
		return 0;
	}

	private class MaintVariables {
		long propogations = 0, rotations = 0;
	}

	private final MaintVariables vars = new MaintVariables();

	private static class Node<K, V> {
		K key;

		volatile V value;
		volatile Node<K, V> left;
		volatile Node<K, V> right;
		final ReentrantLock lock;
		volatile boolean removed;
		volatile Node<K, V> parent;
		volatile AtomicInteger counter;

		Node(final K key, final V value) {
			this.key = key;
			this.value = value;
			this.removed = false;
			this.lock = new ReentrantLock(useFairLocks);
			this.right = null;
			this.left = null;
			this.parent = null;
			this.counter = new AtomicInteger(0);
		}

		Node(final K key, final V value, final Node<K, V> left, final Node<K, V> right, Node<K, V> parent) {
			this.key = key;
			this.value = value;
			this.left = left;
			this.right = right;
			this.lock = new ReentrantLock(useFairLocks);
			this.removed = false;
			this.parent = parent;
			this.counter = new AtomicInteger(0);
		}
	}

	// state
	private final Node<K, V> root = new Node<K, V>(null, null);
	private Comparator<? super K> comparator;
	volatile AtomicInteger counter = new AtomicInteger(0);
	volatile boolean stop = false;
	// used in the getSize function
	int size;
	private long structMods = 0;

	@Override
	public boolean isEmpty() {
		// the structure is all empty including logically remote nodes
		return root.left == null && root.right == null;
	}

	void InitParamsFromEnv() {
		String value = System.getenv("THREAD_NUM");
		if (value != null) {
			threadNum = Integer.parseInt(value);
		}

		splayProb = 1.0 / (Integer.parseInt(System.getenv("INV_SPLAY_PROB")) * threadNum);

		value = System.getenv("K1");
		if (value != null) {
			k1 = Double.parseDouble(value);
		}

		value = System.getenv("K2");
		if (value != null) {
			k2 = Double.parseDouble(value);
		}

		value = System.getenv("MAX_DEPTH");
		if (value != null) {
			maxDepth = Integer.parseInt(value);
		}
	}

	// Constructors
	public LockBasedFriendlyTreeMapSplay() {
		InitParamsFromEnv();
		// temporary
	}

	public LockBasedFriendlyTreeMapSplay(final Comparator<? super K> comparator) {
		InitParamsFromEnv();
		// temporary
		this.comparator = comparator;
	}

	// What is this?
	private Comparable<? super K> comparable(final Object key) {
		if (key == null) {
			throw new NullPointerException();
		}
		if (comparator == null) {
			return (Comparable<? super K>) key;
		}
		return new Comparable<K>() {
			final Comparator<? super K> _cmp = comparator;

			@SuppressWarnings("unchecked")
			public int compareTo(final K rhs) {
				return _cmp.compare((K) key, rhs);
			}
		};
	}

	@Override
	public boolean containsKey(Object key) {
		if (get(key) == null) {
			return false;
		}
		return true;
	}

	public boolean contains(Object key) {
		if (get(key) == null) {
			return false;
		}
		return true;
	}

	void finishCount(int nodesTraversed) {
		Vars vars = counts.get();
		vars.getCount++;
		vars.nodesTraversed += nodesTraversed;
	}

	@Override
	public V get(final Object key) {
		Node<K, V> next, current;
		next = root;
		final Comparable<? super K> k = comparable(key);
		int rightCmp;

		int nodesTraversed = 0;
		long depth = 0;

		while (true) {
			current = next;
			if (current.key == null) {
				rightCmp = -100;
			} else {
				rightCmp = k.compareTo(current.key);
			}
			if (rightCmp == 0) {
				V value = current.value;
				if (value == DELETED) {
					if (TRAVERSAL_COUNT) {
						finishCount(nodesTraversed);
					}
					return null;
				}
				splay(current, depth);
				if (TRAVERSAL_COUNT) {
					finishCount(nodesTraversed);
				}
				return value;
			}
			depth++;
			if (rightCmp <= 0) {
				next = current.left;
			} else {
				next = current.right;
			}
			if (TRAVERSAL_COUNT) {
				nodesTraversed++;
			}
			if (next == null) {
				if (TRAVERSAL_COUNT) {
					finishCount(nodesTraversed);
				}
				return null;
			}
		}
	}

	@Override
	public V remove(final Object key) {
		Node<K, V> next, current;
		next = root;
		final Comparable<? super K> k = comparable(key);
		int rightCmp;
		V value;

		while (true) {
			current = next;
			if (current.key == null) {
				rightCmp = -100;
			} else {
				rightCmp = k.compareTo(current.key);
			}
			if (rightCmp == 0) {
				if (current.value == DELETED) {
					return null;
				}
				current.lock.lock();
				if (!current.removed) {
					break;
				} else {
					current.lock.unlock();
				}
			}
			if (rightCmp <= 0) {
				next = current.left;
			} else {
				next = current.right;
			}
			if (next == null) {
				if (rightCmp != 0) {
					return null;
				}
				// this only happens if node is removed, so you take the
				// opposite path
				// this should never be null
				System.out.println("Going right");
				next = current.right;
			}
		}
		value = current.value;
		if (value == DELETED) {
			current.lock.unlock();
			return null;
		} else {
			current.value = DELETED;
			current.lock.unlock();
			// System.out.println("delete");
			return value;
		}
	}

	@Override
	public V putIfAbsent(K key, V value) {
		int rightCmp;
		Node<K, V> next, current;
		next = root;
		final Comparable<? super K> k = comparable(key);
		Node<K, V> n = null;
		// int traversed = 0;
		V val;

		while (true) {
			current = next;
			// traversed++;
			if (current.key == null) {
				rightCmp = -100;
			} else {
				rightCmp = k.compareTo(current.key);
			}
			if (rightCmp == 0) {
				val = current.value;
				if (val != DELETED) {
					// System.out.println(traversed);
					return val;
				}
				current.lock.lock();
				if (!current.removed) {
					break;
				} else {
					current.lock.unlock();
				}
			}
			if (rightCmp <= 0) {
				next = current.left;
			} else {
				next = current.right;
			}
			if (next == null) {
				if (n == null && allocateOutside) {
					n = new Node<K, V>(key, value);
				}
				current.lock.lock();
				if (!current.removed) {
					if (rightCmp <= 0) {
						next = current.left;
					} else {
						next = current.right;
					}
					if (next == null) {
						break;
					} else {
						current.lock.unlock();
					}
				} else {
					current.lock.unlock();
					// maybe have to check if the other one is still null before
					// going the opposite way?
					// YES!! We do this!
					if (rightCmp <= 0) {
						next = current.left;
					} else {
						next = current.right;
					}
					if (next == null) {
						if (rightCmp > 0) {
							next = current.left;
						} else {
							next = current.right;
						}
					}
				}
			}
		}
		val = current.value;
		if (rightCmp == 0) {
			if (val == DELETED) {
				current.value = value;
				current.lock.unlock();
				// System.out.println("insert");
				// System.out.println(traversed);
				return null;
			} else {
				current.lock.unlock();
				return val;
			}
		} else {
			if (!allocateOutside) {
				n = new Node<K, V>(key, value);
			}
			n.parent = current;
			if (rightCmp <= 0) {
				current.left = n;
			} else {
				current.right = n;
			}
			current.lock.unlock();
			// System.out.println(traversed);
			// System.out.println("insert");
			return null;
		}
	}

	@Override
	public Set<java.util.Map.Entry<K, V>> entrySet() {
		// TODO Auto-generated method stub
		return null;
	}

	private Node<K,V> lockParent(final Node<K,V> node) {
		Node<K, V> parent = node.parent;
		parent.lock.lock();
		while (node.parent != parent) {
			parent.lock.unlock();
			parent = node.parent;
			parent.lock.lock();
		}
		return parent;
	}

	class LockParentResult {
		public final long conflicts;
		public final Node<K,V> parent;
		public LockParentResult(long conflicts, Node<K,V> parent) {
			this.conflicts = conflicts;
			this.parent = parent;
		}
	}

	private LockParentResult tryLockParent(final Node<K,V> node, long conflicts) {
		for (int tries = 0; tries < 5; tries++, conflicts++) {
			if (conflicts >= 10) {
				return new LockParentResult(0, null);
			}
			Node<K, V> parent = node.parent;
			if (parent.lock.tryLock()) {
				if (node.parent == parent) {
					return new LockParentResult(conflicts, parent);
				}
				parent.lock.unlock();
			}
			counts.get().failedLockAcquire++;
		}
		return new LockParentResult(0, null);
	}

	private boolean splayTryRemove(Node<K, V> n) {
		if (!n.removed && n.value == DELETED && (n.left == null || n.right == null)) {
			return splayRemoveNode(n);
		}
		return false;
	}

	private void splay(Node<K, V> n, long depth) {
		long iterations = 0;
		long conflicts = 0;
		if (ThreadLocalRandom.current().nextDouble() >= rotateProb(iterations, depth)) {
			return;
		}
		int curCounter = counter.incrementAndGet();
		int curNodeCounter = n.counter.incrementAndGet();
		int m = (int)Math.floor(Math.log((double)curCounter / curNodeCounter));
		if (depth <= k1 * m || depth < maxDepth) {
			return;
		}
		n.lock.lock();
		if (n.removed || n == root) {
			n.lock.unlock();
			return;
		}
		LockParentResult res = tryLockParent(n, conflicts);
		if (res.parent == null) {
			n.lock.unlock();
			return;
		}
		conflicts = res.conflicts;
		Node<K, V> parent = res.parent;
		if (splayTryRemove(n)) {
			n.lock.unlock();
			parent.lock.unlock();
			return;
		}
		while (parent != root && depth > k2 * m && depth > maxDepth + 1) {
			res = tryLockParent(parent, conflicts);
			if (res.parent == null) {
				break;
			}
			conflicts = res.conflicts;
			Node<K, V> gParent = res.parent;
			if (splayTryRemove(parent)) {
				gParent.lock.unlock();
				break;
			}
			if (gParent == root) {
				// zig
				splayRotate(n);
				parent.lock.unlock();
				parent = gParent;
				break;
			}
			res = tryLockParent(gParent, conflicts);
			if (res.parent == null) {
				gParent.lock.unlock();
				break;
			}
			conflicts = res.conflicts;
			Node<K, V> ggParent = res.parent;
			if (splayTryRemove(gParent)) {
				gParent.lock.unlock();
				ggParent.lock.unlock();
				break;
			}
			if ((parent.left == n) == (gParent.left == parent)) {
				// zig-zig
				splayRotate(parent);
				splayRotate(n);
			} else {
				// zig-zag
				splayRotate(n);
				splayRotate(n);
			}
			parent.lock.unlock();
			gParent.lock.unlock();
			parent = ggParent;
			iterations++;
			depth -= 2;
			// if (ThreadLocalRandom.current().nextDouble() >= rotateProb(iterations, depth)) {
			// 	break;
			// }
		}
		n.lock.unlock();
		parent.lock.unlock();
	}

	private boolean splayRemoveNode(Node<K, V> n) {
		Node<K, V> parent, child;
		if (n.value != DELETED) {
			return false;
		}
		parent = n.parent;
		if ((child = n.left) != null) {
			if (n.right != null) {
				return false;
			}
		} else {
			child = n.right;
		}
		if (parent.left == n) {
			parent.left = child;
		} else {
			parent.right = child;
		}
		if (child != null) {
			child.parent = parent;
		}
		n.left = parent;
		n.right = parent;
		n.removed = true;
		return true;
	}

	private void splayRotate(Node<K, V> node) {
		if (node.parent.left == node) {
			splayRightRotate(node.parent, node.parent.parent);
		} else {
			splayLeftRotate(node.parent, node.parent.parent);
		}
	}

	// parent, n, l are not null and locked
	private void splayRightRotate(Node<K, V> n, Node<K, V> parent) {
		Node<K, V> l, lr, r, newNode;
		l = n.left;
		lr = l.right;
		r = n.right;
		newNode = new Node<>(n.key, n.value, lr, r, l);
		if (r != null) {
			r.parent = newNode;
		}
		if (lr != null) {
			lr.parent = newNode;
		}
		l.right = newNode;
		if (parent.left == n) {
			parent.left = l;
		} else {
			parent.right = l;
		}
		l.parent = parent;
		n.removed = true;
		if (STRUCT_MODS) {
			vars.rotations++;
			counts.get().structMods++;
		}
	}

	// parent, n, r are not null and locked
	private void splayLeftRotate(Node<K, V> n, Node<K, V> parent) {
		Node<K, V> r, rl, l, newNode;
		r = n.right;
		rl = r.left;
		l = n.left;
		newNode = new Node<>(n.key, n.value, l, rl, r);
		if (l != null) {
			l.parent = newNode;
		}
		if (rl != null) {
			rl.parent = newNode;
		}
		r.left = newNode;

		n.right = parent;
		n.left = parent;

		if (parent.left == n) {
			parent.left = r;
		} else {
			parent.right = r;
		}
		r.parent = parent;
		n.removed = true;
		if (STRUCT_MODS) {
			vars.rotations++;
			counts.get().structMods++;
		}
	}

	// not thread safe
	public int getSize() {
		this.size = 0;
		recursiveGetSize(root.left);
		return size;
	}

	void recursiveGetSize(Node<K, V> node) {
		if (node == null)
			return;
		if (node.removed) {
			// System.out.println("Shouldn't find removed nodes in the get size function");
		}
		if (node.value != DELETED) {
			this.size++;
		}
		recursiveGetSize(node.left);
		recursiveGetSize(node.right);
	}

	public int numNodes() {
		this.size = 0;
		ConcurrentHashMap<Integer, Node<K, V>> map = new ConcurrentHashMap<Integer, Node<K, V>>();
		recursiveNumNodes(root.left, map);
		return size;
	}

	void recursiveNumNodes(Node<K, V> node,
			ConcurrentHashMap<Integer, Node<K, V>> map) {
		if (node == null)
			return;
		if (node.removed) {
			// System.out.println("Shouldn't find removed nodes in the get size function");
		}
		Node<K, V> n = map.putIfAbsent((Integer) node.key, node);
		if (n != null) {
			System.out.println("Error: " + node.key);
		}
		this.size++;
		recursiveNumNodes(node.left, map);
		recursiveNumNodes(node.right, map);
	}

	public int getBalance() {
		int lefth = 0, righth = 0;
		if (root.left == null)
			return 0;
		lefth = recursiveDepth(root.left.left);
		righth = recursiveDepth(root.left.right);
		return lefth - righth;
	}

	int recursiveDepth(Node<K, V> node) {
		if (node == null) {
			return 0;
		}
		int lefth, righth;
		lefth = recursiveDepth(node.left);
		righth = recursiveDepth(node.right);
		return Math.max(lefth, righth) + 1;
	}

	@Override
	public void clear() {
		this.structMods = 0;
		this.vars.propogations = 0;
		this.vars.rotations = 0;
		root.left = null;
	}

	@Override
	public int size() {
		return this.getSize();
	}

	public long getStructMods() {
		return structMods;
	}

}
