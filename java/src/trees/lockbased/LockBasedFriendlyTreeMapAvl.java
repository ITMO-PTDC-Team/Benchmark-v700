package trees.lockbased;

import java.util.AbstractMap;
import java.util.Comparator;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ThreadLocalRandom;
import java.util.concurrent.locks.ReentrantLock;

import contention.abstractions.CompositionalMap;
import contention.abstractions.CompositionalMap.Vars;
import contention.abstractions.MaintenanceAlg;

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

public class LockBasedFriendlyTreeMapAvl<K, V> extends AbstractMap<K, V> implements
		CompositionalMap<K, V>, MaintenanceAlg {

	static final boolean useFairLocks = false;
	// we encode directions as characters
	static final char Left = 'L';
	static final char Right = 'R';
	final V DELETED = (V) new Object();

	private static int height(final Node<?, ?> node) {
		return node == null ? 0 : node.height;
	}

	static final boolean STRUCT_MODS = true;
	static final double SPLAY_PROB = 1.0 / 1;
	final static int CONFLICTS = 500;
	static final long SPIN_COUNT = 100;
	final static int THREAD_NUM = 8;
	final static int MAX_DEPTH = 1;

	private static final int UnlinkRequired = -1;
	private static final int RebalanceRequired = -2;
	private static final int NothingRequired = -3;

	private static class Node<K, V> {
		K key;

		volatile V value;
		volatile Node<K, V> left;
		volatile Node<K, V> right;
		final ReentrantLock lock;
		volatile boolean removed;
		volatile Node<K, V> parent;
		volatile int height;

		Node(final K key, final V value) {
			this.key = key;
			this.value = value;
			this.removed = false;
			this.lock = new ReentrantLock(useFairLocks);
			this.right = null;
			this.left = null;
			this.height = 1;
			this.parent = null;
		}

		Node(final K key, final V value, final Node<K, V> left, final Node<K, V> right, Node<K, V> parent) {
			this.key = key;
			this.height = 0;
			this.value = value;
			this.left = left;
			this.right = right;
			this.lock = new ReentrantLock(useFairLocks);
			this.removed = false;
			this.parent = parent;
		}

		Node(final K key, final int height, final V value, final Node<K, V> left, final Node<K, V> right) {
			this.key = key;
			this.height = height;
			this.value = value;
			this.left = left;
			this.right = right;
			this.lock = new ReentrantLock(useFairLocks);
			this.removed = false;
			this.parent = null;
		}

		void setupNode(final K key, final int height, final V value, final Node<K, V> left, final Node<K, V> right) {
			this.key = key;
			this.height = height;
			this.value = value;
			this.left = left;
			this.right = right;
			this.removed = false;
			this.parent = null;
		}
	}

	// state
	private final Node<K, V> root = new Node<K, V>(null, null);
	private Comparator<? super K> comparator;
	volatile boolean stop = false;
	// used in the getSize function
	int size;
	private long structMods = 0;

	@Override
	public boolean isEmpty() {
		// the structure is all empty including logically remote nodes
		return root.left == null && root.right == null;
	}

	// Constructors
	public LockBasedFriendlyTreeMapAvl() {
	}

	public LockBasedFriendlyTreeMapAvl(final Comparator<? super K> comparator) {
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
				if (TRAVERSAL_COUNT) {
					finishCount(nodesTraversed);
				}
				return value;
			}
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
				if (n == null) {
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
			if (rightCmp <= 0) {
				current.left = n;
			} else {
				current.right = n;
			}
			n.parent = current;
			current.lock.unlock();
			fixHeightAndRebalance(current);
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

	private int nodeCondition(final Node<K, V> node) {
		// Begin atomic.

		final Node<K, V> nL = node.left;
		final Node<K, V> nR = node.right;

		if ((nL == null || nR == null) && node.value == DELETED) {
			return UnlinkRequired;
		}

		final int hN = node.height;
		final int hL0 = height(nL);
		final int hR0 = height(nR);

		// End atomic. Since any thread that changes a node promises to fix
		// it, either our read was consistent (and a NothingRequired conclusion
		// is correct) or someone else has taken responsibility for either node
		// or one of its children.

		final int hNRepl = 1 + Math.max(hL0, hR0);
		final int bal = hL0 - hR0;

		if (bal < -1 || bal > 1) {
			return RebalanceRequired;
		}

		return hN != hNRepl ? hNRepl : NothingRequired;
	}

	/**
	 * Attempts to fix the height of a (locked) damaged node, returning the
	 * lowest damaged node for which this thread is responsible. Returns null if
	 * no more repairs are needed.
	 */
	private Node<K, V> fixHeight_nl(final Node<K, V> node) {
		final int c = nodeCondition(node);
		switch (c) {
		case RebalanceRequired:
		case UnlinkRequired:
			// can't repair
			return node;
		case NothingRequired:
			// Any future damage to this node is not our responsibility.
			return null;
		default:
			node.height = c;
			// we've damaged our parent, but we can't fix it now
			return node.parent;
		}
	}

	/** Does not adjust the size or any heights. */
	private boolean attemptUnlink_nl(final Node<K, V> parent, final Node<K, V> node) {
		// assert (Thread.holdsLock(parent));
		// assert (Thread.holdsLock(node));
		assert (!parent.removed);

		final Node<K, V> parentL = parent.left;
		final Node<K, V> parentR = parent.right;
		if (parentL != node && parentR != node) {
			// node is no longer a child of parent
			return false;
		}

		assert (!node.removed);
		assert (parent == node.parent);

		final Node<K, V> left = node.left;
		final Node<K, V> right = node.right;
		if (left != null && right != null) {
			// splicing is no longer possible
			return false;
		}
		final Node<K, V> splice = left != null ? left : right;

		if (parentL == node) {
			parent.left = splice;
		} else {
			parent.right = splice;
		}
		if (splice != null) {
			splice.parent = parent;
		}

		node.removed = true;
		node.value = DELETED;

		return true;
	}

	private Node<K, V> rotateRight_nl(final Node<K, V> nParent,
			final Node<K, V> n, final Node<K, V> nL, final int hR,
			final int hLL, final Node<K, V> nLR, final int hLR) {
		if (STRUCT_MODS)
			counts.get().structMods += 1;

		final int hNRepl = 1 + Math.max(hLR, hR);

		final Node<K, V> nPL = nParent.left;

		Node<K, V> newNode = new Node<K, V>(null, null);
		newNode.setupNode(
			n.key,
			hNRepl,
			n.value,
			nLR,
			n.right
		);
		newNode.parent = nL;
		if (nLR != null) {
			nLR.parent = newNode;
		}
		if (newNode.right != null) {
			newNode.right.parent = newNode;
		}

		nL.right = newNode;
		if (nPL == n) {
			nParent.left = nL;
		} else {
			nParent.right = nL;
		}
		nL.parent = nParent;

		nL.height = 1 + Math.max(hLL, hNRepl);

		// We have damaged nParent, n (now parent.child.right), and nL (now
		// parent.child). n is the deepest. Perform as many fixes as we can
		// with the locks we've got.

		// We've already fixed the height for n, but it might still be outside
		// our allowable balance range. In that case a simple fixHeight_nl
		// won't help.

		n.removed = true;

		final int balN = hLR - hR;
		if (balN < -1 || balN > 1) {
			// we need another rotation at n
			return newNode;
		}

		// we've already fixed the height at nL, do we need a rotation here?
		final int balL = hLL - hNRepl;
		if (balL < -1 || balL > 1) {
			return nL;
		}

		// try to fix the parent height while we've still got the lock
		return fixHeight_nl(nParent);
	}

	private Node<K, V> rotateLeft_nl(final Node<K, V> nParent,
			final Node<K, V> n, final int hL, final Node<K, V> nR,
			final Node<K, V> nRL, final int hRL, final int hRR) {
		if (STRUCT_MODS)
			counts.get().structMods += 1;

		final int hNRepl = 1 + Math.max(hL, hRL);

		final Node<K, V> nPL = nParent.left;

		Node<K, V> newNode = new Node<K, V>(null, null);
		newNode.setupNode(
			n.key,
			hNRepl,
			n.value,
			n.left,
			nRL
		);
		newNode.parent = nR;
		if (nRL != null) {
			nRL.parent = newNode;
		}
		if (newNode.left != null) {
			newNode.left.parent = newNode;
		}

		nR.left = newNode;
		if (nPL == n) {
			nParent.left = nR;
		} else {
			nParent.right = nR;
		}
		nR.parent = nParent;

		nR.height = 1 + Math.max(hNRepl, hRR);

		n.removed = true;

		final int balN = hRL - hL;
		if (balN < -1 || balN > 1) {
			return newNode;
		}

		final int balR = hRR - hNRepl;
		if (balR < -1 || balR > 1) {
			return nR;
		}

		return fixHeight_nl(nParent);
	}

	private Node<K, V> rotateRightOverLeft_nl(final Node<K, V> nParent,
			final Node<K, V> n, final Node<K, V> nL, final int hR,
			final int hLL, final Node<K, V> nLR, final int hLRL) {
		final Node<K, V> nPL = nParent.left;
		final Node<K, V> nLRL = nLR.left;
		final Node<K, V> nLRR = nLR.right;
		final int hLRR = height(nLRR);

		final int hNRepl = 1 + Math.max(hLRR, hR);
		final int hLRepl = 1 + Math.max(hLL, hLRL);

		if (STRUCT_MODS)
			counts.get().structMods += 1;

		Node<K, V> newN = new Node<K, V>(null, null);
		newN.setupNode(
			n.key,
			hNRepl,
			n.value,
			nLRR,
			n.right
		);
		newN.parent = nLR;
		if (newN.left != null) {
			newN.left.parent = newN;
		}
		if (newN.right != null) {
			newN.right.parent = newN;
		}

		Node<K, V> newNL = new Node<K, V>(null, null);
		newNL.setupNode(
			nL.key,
			hLRepl,
			nL.value,
			nL.left,
			nLRL
		);
		newNL.parent = nLR;
		if (newNL.left != null) {
			newNL.left.parent = newNL;
		}
		if (newNL.right != null) {
			newNL.right.parent = newNL;
		}
		nLR.left = newNL;
		nLR.right = newN;
		if (nPL == n) {
			nParent.left = nLR;
		} else {
			nParent.right = nLR;
		}
		nLR.parent = nParent;

		nLR.height = 1 + Math.max(hLRepl, hNRepl);

		assert (Math.abs(hLL - hLRL) <= 1);

		n.removed = true;
		nL.removed = true;

		final int balN = hLRR - hR;
		if (balN < -1 || balN > 1) {
			return newN;
		}

		final int balLR = hLRepl - hNRepl;
		if (balLR < -1 || balLR > 1) {
			return nLR;
		}

		return fixHeight_nl(nParent);
	}

	private Node<K, V> rotateLeftOverRight_nl(final Node<K, V> nParent,
			final Node<K, V> n, final int hL, final Node<K, V> nR,
			final Node<K, V> nRL, final int hRR, final int hRLR) {
		final Node<K, V> nPL = nParent.left;
		final Node<K, V> nRLL = nRL.left;
		final int hRLL = height(nRLL);
		final Node<K, V> nRLR = nRL.right;

		if (STRUCT_MODS)
			counts.get().structMods += 1;

		final int hNRepl = 1 + Math.max(hL, hRLL);
		final int hRRepl = 1 + Math.max(hRLR, hRR);

		Node<K, V> newN = new Node<K, V>(null, null);
		newN.setupNode(
			n.key,
			hNRepl,
			n.value,
			n.left,
			nRLL
		);
		newN.parent = nRL;
		if (newN.left != null) {
			newN.left.parent = newN;
		}
		if (newN.right != null) {
			newN.right.parent = newN;
		}

		Node<K, V> newNR = new Node<K, V>(null, null);
		newNR.setupNode(
			nR.key,
			hRRepl,
			nR.value,
			nRLR,
			nR.right
		);
		newNR.parent = nRL;
		if (newNR.left != null) {
			newNR.left.parent = newNR;
		}
		if (newNR.right != null) {
			newNR.right.parent = newNR;
		}

		nRL.right = newNR;
		nRL.left = newN;
		if (nPL == n) {
			nParent.left = nRL;
		} else {
			nParent.right = nRL;
		}
		nRL.parent = nParent;

		// fix up heights
		nRL.height = 1 + Math.max(hNRepl, hRRepl);

		assert (Math.abs(hRR - hRLR) <= 1);

		n.removed = true;
		nR.removed = true;

		final int balN = hRLL - hL;
		if (balN < -1 || balN > 1) {
			return newN;
		}
		final int balRL = hRRepl - hNRepl;
		if (balRL < -1 || balRL > 1) {
			return nRL;
		}
		return fixHeight_nl(nParent);
	}

	private Node<K, V> rebalanceToRight_nl(final Node<K, V> nParent, final Node<K, V> n, final Node<K, V> nL, final int hR0) {
		// L is too large, we will rotate-right. If L.R is taller
		// than L.L, then we will first rotate-left L.
		nL.lock.lock();
		try {
			final int hL = nL.height;
			if (hL - hR0 <= 1) {
				return n; // retry
			} else {
				final Node<K, V> nLR = nL.right;
				final int hLL0 = height(nL.left);
				final int hLR0 = height(nLR);
				if (hLL0 >= hLR0) {
					// rotate right based on our snapshot of hLR
					return rotateRight_nl(nParent, n, nL, hR0, hLL0, nLR, hLR0);
				} else {
					nLR.lock.lock();
					try {
						// If our hLR snapshot is incorrect then we might
						// actually need to do a single rotate-right on n.
						final int hLR = nLR.height;
						if (hLL0 >= hLR) {
							return rotateRight_nl(nParent, n, nL, hR0, hLL0,
									nLR, hLR);
						} else {
							// If the underlying left balance would not be
							// sufficient to actually fix n.left, then instead
							// of rolling it into a double rotation we do it on
							// it's own. This may let us avoid rotating n at
							// all, but more importantly it avoids the creation
							// of damaged nodes that don't have a direct
							// ancestry relationship. The recursive call to
							// rebalanceToRight_nl in this case occurs after we
							// release the lock on nLR.

							final int hLRL = height(nLR.left);
							final int b = hLL0 - hLRL;
							if (b >= -1 && b <= 1) {
								// nParent.child.left won't be damaged after a
								// double rotation
								return rotateRightOverLeft_nl(nParent, n, nL,
										hR0, hLL0, nLR, hLRL);
							}
						}
					} finally {
						nLR.lock.unlock();
					}
					// focus on nL, if necessary n will be balanced later
					return rebalanceToLeft_nl(n, nL, nLR, hLL0);
				}
			}
		} finally {
			nL.lock.unlock();
		}
	}

	private Node<K, V> rebalanceToLeft_nl(final Node<K, V> nParent,
			final Node<K, V> n, final Node<K, V> nR, final int hL0) {
		nR.lock.lock();
		try {
			final int hR = nR.height;
			if (hL0 - hR >= -1) {
				return n; // retry
			} else {
				final Node<K, V> nRL = nR.left;
				final int hRL0 = height(nRL);
				final int hRR0 = height(nR.right);
				if (hRR0 >= hRL0) {
					return rotateLeft_nl(nParent, n, hL0, nR, nRL, hRL0, hRR0);
				} else {
					nRL.lock.lock();
					try {
						final int hRL = nRL.height;
						if (hRR0 >= hRL) {
							return rotateLeft_nl(nParent, n, hL0, nR, nRL, hRL,
									hRR0);
						} else {
							final int hRLR = height(nRL.right);
							final int b = hRR0 - hRLR;
							if (b >= -1 && b <= 1) {
								return rotateLeftOverRight_nl(nParent, n, hL0,
										nR, nRL, hRR0, hRLR);
							}
						}
					} finally {
						nRL.lock.unlock();
					}
					return rebalanceToRight_nl(n, nR, nRL, hRR0);
				}
			}
		} finally {
			nR.lock.unlock();
		}
	}

	/**
	 * nParent and n must be locked on entry. Returns a damaged node, or null if
	 * no more rebalancing is necessary.
	 */
	private Node<K, V> rebalance_nl(final Node<K, V> nParent, final Node<K, V> n) {
		final Node<K, V> nL = n.left;
		final Node<K, V> nR = n.right;

		if ((nL == null || nR == null) && n.value == DELETED) {
			if (attemptUnlink_nl(nParent, n)) {
				// attempt to fix nParent.height while we've still got the lock
				return fixHeight_nl(nParent);
			} else {
				// retry needed for n
				return n;
			}
		}

		final int hN = n.height;
		final int hL0 = height(nL);
		final int hR0 = height(nR);
		final int hNRepl = 1 + Math.max(hL0, hR0);
		final int bal = hL0 - hR0;

		if (bal > 1) {
			return rebalanceToRight_nl(nParent, n, nL, hR0);
		} else if (bal < -1) {
			return rebalanceToLeft_nl(nParent, n, nR, hL0);
		} else if (hNRepl != hN) {
			// we've got more than enough locks to do a height change, no need
			// to
			// trigger a retry
			n.height = hNRepl;

			// nParent is already locked, let's try to fix it too
			return fixHeight_nl(nParent);
		} else {
			// nothing to do
			return null;
		}
	}

	private void fixHeightAndRebalance(Node<K, V> node) {
		Node<K, V> next = null;
		while (node != null && node.parent != null) {
			final int condition = nodeCondition(node);
			if (condition == NothingRequired || node.removed) {
				// nothing to do, or no point in fixing this node
				return;
			}

			next = node;
			if (condition != UnlinkRequired && condition != RebalanceRequired) {
				node.lock.lock();
				next = fixHeight_nl(node);
				node.lock.unlock();
			} else {
				final Node<K, V> nParent = node.parent;
				nParent.lock.lock();
				if (!nParent.removed && !node.removed && node.parent == nParent) {
					node.lock.lock();
					next = rebalance_nl(nParent, node);
					node.lock.unlock();
				}
				nParent.lock.unlock();
				// else RETRY
			}
			node = next;
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
			System.out.println("Shouldn't find removed nodes in the get size function");
		}
		if (node.value != DELETED) {
			this.size++;
		}
		recursiveGetSize(node.left);
		recursiveGetSize(node.right);
	}

	@Override
	public void clear() {
		this.resetTree();

		return;
	}

	private void resetTree() {
		this.structMods = 0;
		root.left = null;
	}

	@Override
	public int size() {
		return this.getSize();
	}

	public long getStructMods() {
		return structMods;
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
		node.lock.lock();
		if (node.removed) {
			System.out.println("Shouldn't find removed nodes in the get size function");
		}
		Node<K, V> n = map.putIfAbsent((Integer) node.key, node);
		if (n != null) {
			System.out.println("Error: " + node.key);
		}
		this.size++;
		recursiveNumNodes(node.left, map);
		recursiveNumNodes(node.right, map);
		node.lock.unlock();
	}



	public boolean stopMaintenance() {
		return true;
	}
}
