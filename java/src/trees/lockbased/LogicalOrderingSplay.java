package trees.lockbased;

import java.util.AbstractMap;
import java.util.AbstractSet;
import java.util.Comparator;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.ThreadLocalRandom;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import contention.abstractions.CompositionalMap;

/**
 * Implementation of concurrent AVL tree based on the paper 
 * "Practical Concurrent Binary Search Trees via Logical Ordering" by 
 * Dana Drachsler (Technion), Martin Vechev (ETH) and Eran Yahav (Technion).
 *
 * Copyright 2013 Dana Drachsler (ddana [at] cs [dot] technion [dot] ac [dot] il).
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * @author Dana Drachsler
 */
public class LogicalOrderingSplay<K, V> extends AbstractMap<K,V> implements ConcurrentMap<K,V>, CompositionalMap<K, V> {

	/** The tree's root */
	private MapNode<K,V> root;
	
	public enum RebalanceMode {
		None,
		Splay,
	}
	/** The keys' comparator */
	private Comparator<? super K> comparator;

	final static RebalanceMode REBALANCE_MODE = RebalanceMode.Splay;
	final static int CONFLICTS = 500;
	final static int SPIN_COUNT = 100;
	final static int MAX_DEPTH = 5;
	final static boolean STRUCT_MODS = true;
	static final double SPLAY_PROB = 1.0 / 20000;
	final static int THREAD_NUM = 16;

	/** A constant object for the use of the {@code insert} method.  */
	private final static Object EMPTY_ITEM = new Object();


	private static double rotateProb(final long depth, final long iterations) {
		if (iterations == 0) {
			if (depth > MAX_DEPTH) {
				return 1.0 / (1024 * THREAD_NUM);
			}
			return 1.0 / (1024 * 1024 * THREAD_NUM);
		}
		if (depth == MAX_DEPTH || depth == MAX_DEPTH - 1) {
			return 0;
		}
		return 1.0;
	}

	public LogicalOrderingSplay() {
		MapNode parent = new MapNode(Integer.MIN_VALUE);
		root = new MapNode(Integer.MAX_VALUE, null, parent, parent, parent);
		root.parent = parent;
		parent.right = root;
		parent.succ = root;
	}
	
	/**
	 * Constructor, initialize the tree and the logical ordering layouts.
	 * The logical ordering is initialized by creating two nodes, where their 
	 * keys are the minimal and maximal values. 
	 * The tree layout is initialized by setting the root to point to the node 
	 * with the maximal value.
	 * 
	 * @param min The minimal value
	 * @param max The maximal value
	 */
	public LogicalOrderingSplay(final K min, final K max) {
		MapNode<K,V> parent = new MapNode<K,V>(min);
		root = new MapNode<K, V>(max, null, parent, parent, parent);
		root.parent = parent;
		parent.right = root;
		parent.succ = root;
	}

	/**
	 * Constructor, initialize the tree and the logical ordering layouts.
	 * The logical ordering is initialized by creating two nodes, where their 
	 * keys are the minimal and maximal values. 
	 * The tree layout is initialized by setting the root to point to the node 
	 * with the maximal value.
	 * 
	 * @param min The minimal value
	 * @param max The maximal value
	 * @param comparator The keys' comparator
	 */
	public LogicalOrderingSplay(K min, K max, Comparator<? super K> comparator) {
		this(min, max);
		this.comparator = comparator;
	}
	
	/**
	 * Given some object, returns an appropriate {@link Comparable} object.
	 * If the comparator was initialized upon creating the tree, the 
	 * {@link Comparable} object uses it; otherwise, assume that the given 
	 * object implements {@link Comparable}.
	 *  
	 * @param object The object 
	 * @return The appropriate {@link Comparable} object
	 */
	@SuppressWarnings("unchecked")
	private Comparable<? super K> comparable(final Object object) {

		if (object == null) throw new NullPointerException();
		if (comparator == null) return (Comparable<? super K>)object;

		return new Comparable<K>() {
			final Comparator<? super K> compar = comparator;
			final K obj = (K) object;

			public int compareTo(final K other) { 
				return compar.compare(obj, other); 
			}
		};
	}

	void finishCount(int treeNodesTraversed, int logicalNodesTraversed, boolean found) {
		Vars vars = counts.get();
		vars.getCount++;
		if (found) {
			vars.foundCnt++;
			vars.foundTreeTraversed += treeNodesTraversed;
			vars.foundLogicalTraversed += logicalNodesTraversed;
		} else {
			vars.notFoundCnt++;
			vars.notFoundTreeTraversed += treeNodesTraversed;
			vars.notFoundLogicalTraversed += logicalNodesTraversed;
		}
	}
	
	/**
	 * Traverses the tree to find a node with the given key.
	 * 
	 * @see java.util.Map#get(java.lang.Object)
	 */
	final public V get(final Object key) {
		final Comparable<? super K> value = comparable(key);

		int treeTraversed = 0;

		MapNode<K,V> node = root;
		MapNode<K,V> child;
		K val;
		int res = -1;
		int depth = 0;
		while (true) {
			if (res == 0) break;
			if (res > 0) {
				child = node.right;
			} else {
				child = node.left;
			}
			depth++;
			if (TRAVERSAL_COUNT) {
				treeTraversed++;
			}
			if (child == null) break;
			node = child;
			val = node.key;
			res = value.compareTo(val);
		}

		int logicalTraversed = 0;
		boolean logical = false;
		boolean a = false;
		while (res < 0) {
			logical = true;
			a = true;
			node = node.pred;
			val =  node.key;
			res = value.compareTo(val);
			if (TRAVERSAL_COUNT) {
				logicalTraversed++;
			}
		}
		while (!a && res > 0) {
			logical = true;
			node = node.succ;
			val =  node.key;
			res = value.compareTo(val);
			if (TRAVERSAL_COUNT) {
				logicalTraversed++;
			}
		}
		depth = logical ? 0 : depth;
		if (res == 0 && node.valid) {
			if (TRAVERSAL_COUNT) {
				finishCount(treeTraversed, logicalTraversed, true);
			}
			if (REBALANCE_MODE == RebalanceMode.Splay) {
				splay(node, depth);
			}
			return (V) node.item;
		}
		if (TRAVERSAL_COUNT) {
			finishCount(treeTraversed, logicalTraversed, false);
		}
		return null;
	}
	
	/**
	 * Traverses the tree to find a node with the given key.
	 * 
	 * @see java.util.Map#containsKey(java.lang.Object)
	 */
	@Override
	final public boolean containsKey(final Object key) {
		final Comparable<? super K> value = comparable(key);

		int treeTraversed = 0;

		MapNode<K,V> node = root;
		MapNode<K,V> child;
		int res = -1;
		K val;
		int depth = 0;
		while (true) {
			if (res == 0) break;
			if (res > 0) {
				child = node.right;
			} else {
				child = node.left;
			}
			if (TRAVERSAL_COUNT) {
				treeTraversed++;
			}
			depth++;
			if (child == null) break;
			node = child;
			val = node.key;
			res = value.compareTo(val);
		}
		int logicalTraversed = 0;
		boolean logical = false;
		boolean a = false;
		while (res < 0) {
			a = true;
			logical = true;
			node = node.pred;
			val =  node.key;
			res = value.compareTo(val);
			if (TRAVERSAL_COUNT) {
				logicalTraversed++;
			}
		}
		while (!a && res > 0) {
			logical = true;
			node = node.succ;
			val =  node.key;
			res = value.compareTo(val);
			if (TRAVERSAL_COUNT) {
				logicalTraversed++;
			}
		}
		depth = logical ? 0 : depth;
		if (res == 0 && node.valid) {
			if (REBALANCE_MODE == RebalanceMode.Splay) {
				splay(node, depth);
			}
		}
		if (TRAVERSAL_COUNT) {
			finishCount(treeTraversed, logicalTraversed, res == 0 && node.valid);
		}
		return (res == 0 && node.valid);
	}
	
	/**
	 * @see java.util.Map#put(java.lang.Object, java.lang.Object)
	 */
	@Override
	public V put(K key, V value) {
		return insert(key, value, false, false, null);
	}
	
	/**
	 * @see java.util.concurrent.ConcurrentMap#putIfAbsent(java.lang.Object, java.lang.Object)
	 */
	@Override
	public V putIfAbsent(K key, V value) {
		return insert(key, value, true, false, null);
	}
	
	/**
	 * @see java.util.concurrent.ConcurrentMap#replace(java.lang.Object, java.lang.Object)
	 */
	@Override
	public V replace(K key, V value) {
		return insert(key, value, false, true, EMPTY_ITEM);
	}

	/**
	 * @see java.util.concurrent.ConcurrentMap#replace(java.lang.Object, java.lang.Object, java.lang.Object)
	 */
	@Override
	public boolean replace(K key, V oldValue, V newValue) {
		return insert(key, newValue, false, true, oldValue).equals(oldValue);
	}

	/**
	 * Insert the pair (key, item) to the tree.
	 * If the key is already present, update the item if putIfAbsent equals {@code false}.
	 * If {@code isReplace} equals {@code true}, the operation takes place only 
	 * if the key is already present. Before applying the replacement, the 
	 * operation considers the {@code replaceItem}. If this item equals
	 * {@code EmptyItem}, the replacement is applied without considering the 
	 * current item associated with that key. Otherwise, the replacement is 
	 * applied only if the current item equals to {@code replaceItem}.
	 * 
	 * @param key The key
	 * @param item The item
	 * @param putIfAbsent Keep the old item if key is already present?
	 * @param isReplace Is the operation should only take place if the key is already present? 
	 * @param replaceItem The item to consider upon replacement.
	 * @return The item that was associated with the given key, or null if the
	 * key was not present in the tree
	 */
	final private V insert(final K key, final V item, boolean putIfAbsent, boolean isReplace, Object replaceItem) {
		final Comparable<? super K> value = comparable(key);
		MapNode<K,V> node = null;
		K nodeValue = null;
		int res = -1;
		while (true) {
			node = root;
			MapNode<K,V> child;
			res = -1;
			while (true) {
				if (res == 0) break;
				if (res > 0) {
					child = node.right;
				} else {
					child = node.left;
				}
				if (child == null) break;
				node = child;
				nodeValue = node.key;
				res = value.compareTo(nodeValue);
			}
			final MapNode<K,V> pred = res > 0 ? node : node.pred;
			pred.lockSuccLock();
			if (pred.valid) {
				final K predVal = pred.key;
				final int predRes = pred== node? res: value.compareTo(predVal);
				if (predRes > 0) {
					final MapNode<K,V> succ = pred.succ;
					final K succVal = succ.key;
					final int res2 = succ == node? res: value.compareTo(succVal);
					if (res2 <= 0) {
						if (res2 == 0) {
							V item2 = (V) succ.item;
							if (!putIfAbsent && 
									(!isReplace || replaceItem.equals(EMPTY_ITEM) || succ.item.equals(replaceItem))) {
								succ.item = item;
							}
							pred.unlockSuccLock();
							return item2;
						}
						if (isReplace) {
							pred.unlockSuccLock();
							return null;
						}
						final MapNode<K,V> parent = chooseParent(pred, succ, node);
						final MapNode<K,V> newNode = new MapNode<K,V>(key, item, pred, succ, parent);
						succ.pred = newNode;
						pred.succ = newNode;
						pred.unlockSuccLock();
						insertToTree(parent, newNode, parent == pred);
						return null;
					}
				}
			}
			pred.unlockSuccLock();
		}
	}
	
	/**
	 * Choose and lock the correct parent, given the new node's predecessor, 
	 * successor, and the node returned from the traversal.
	 * 
	 * @param pred The predecessor
	 * @param succ The successor
	 * @param firstCand The node returned from the traversal
	 * @return The correct parent
	 */
	final private MapNode<K,V> chooseParent(final MapNode<K,V> pred, 
			final MapNode<K,V> succ, final MapNode<K,V> firstCand) {
		MapNode<K,V> candidate = firstCand == pred || firstCand == succ? firstCand: pred;
		while (true) {
			candidate.lockTreeLock();
			if (candidate == pred) {
				if (candidate.right == null) {
					return candidate;
				}
				candidate.unlockTreeLock();
				candidate = succ;
			} else {
				if (candidate.left == null) {
					return candidate;
				}
				candidate.unlockTreeLock();
				candidate = pred;
			}
			Thread.yield();
		}
	}

	/**
	 * Update the tree layout by connecting the new node to its parent.
	 * Then, the parent's height is updated, and {@link #rebalance} is called.
	 * 
	 * @param parent The new node's parent
	 * @param newNode The new node
	 * @param isRight Is the new node should be the parent's right child?
	 */
	final private void insertToTree(final MapNode<K,V> parent, final MapNode<K,V> newNode, final boolean isRight) {
		if (isRight) {
			parent.right = newNode;
		} else {
			parent.left = newNode;
		}
		parent.unlockTreeLock();
	}

	/**
	 * Lock the given node's parent. 
	 * The operation begins by first reading the node's parent from the node,
	 * then acquiring the parent's lock, and then checking whether this is the 
	 * correct parent. If not, the lock is released, and the operation restarts.
	 * 
	 * @param node The node 
	 * @return The node's parent (which is locked)
	 */
	final private MapNode<K,V> lockParent(final MapNode<K,V> node) {
		MapNode<K, V> parent = node.parent;
		parent.lockTreeLock();
		while (node.parent != parent || !parent.valid) {
			parent.unlockTreeLock();
			parent = node.parent;
			while (!parent.valid) {
				Thread.yield();
				parent = node.parent;
			}
			parent.lockTreeLock();
		}
		return parent;
	}

	class LockParentResult {
		public final int conflicts;
		public final MapNode<K,V> parent;
		public LockParentResult(int conflicts, MapNode<K,V> parent) {
			this.conflicts = conflicts;
			this.parent = parent;
		}
	}

	final private LockParentResult tryLockParent(final MapNode<K,V> node, int conflicts) {
		for (int tries = 0; tries < SPIN_COUNT; tries++, conflicts++) {
			if (conflicts >= CONFLICTS) {
				return new LockParentResult(0, null);
			}
			MapNode<K, V> parent = node.parent;
			if (parent.tryLockTreeLock()) {
				if (node.parent == parent && parent.valid) {
					return new LockParentResult(conflicts, parent);
				}
				parent.unlockTreeLock();
			}
			counts.get().failedLockAcquire++;
		}
		return new LockParentResult(0, null);
	}

	/**
	 * @see java.util.Map#remove(java.lang.Object)
	 */
	@Override
	final public V remove(final Object key) {
		return remove(key, false, null);
	}
	
	/**
	 * @see java.util.concurrent.ConcurrentMap#remove(java.lang.Object, java.lang.Object)
	 */
	@Override
	final public boolean remove(final Object key, final Object item) {
		return remove(key, true, item) != null;
	}
	
	/**
	 * Remove the given key from the tree. 
	 * If the flag {@code compareItem} equals true, remove the key only if the
	 * node is associated with the given item.
	 * 
	 * @param key The key to remove
	 * @param compareItem The flag that indicates whether to consider the given
	 * item
	 * @param item The given item
	 * @return The item of the node that was removed, or null if no node was
	 * removed
	 */
	final public V remove(final Object key, final boolean compareItem, final Object item) {
		Comparable<? super K> value = comparable(key);
		MapNode<K,V> pred, node = null;
		K nodeValue = null;
		int res = 0;
		while (true) {
			node = root;
			MapNode<K,V> child;
			res = -1;
			int nodesTraversed = 0;
			while (true) {
				if (res == 0) break;
				if (res > 0) {
					child = node.right;
				} else {
					child = node.left;
				}
				if (child == null) break;
				node = child;
				nodeValue = node.key;
				res = value.compareTo(nodeValue);
			}
			pred = res > 0 ? node : node.pred;
			pred.lockSuccLock();
			if (pred.valid) {
				final K predVal = pred.key;
				final int predRes = pred== node? res: value.compareTo(predVal);
				if (predRes > 0) {
					MapNode<K,V> succ = pred.succ;
					final K succVal = succ.key;
					int res2 = succ == node? res: value.compareTo(succVal);
					if (res2 <= 0) {
						if (res2 != 0 || (compareItem && !succ.item.equals(item))) {
							pred.unlockSuccLock();
							return null;
						}
						succ.lockSuccLock();
						MapNode<K,V> successor = acquireTreeLocks(succ);
						MapNode<K, V> succParent = lockParent(succ);
						succ.valid = false;
						V succItem = (V) succ.item;
						MapNode<K, V> succSucc = succ.succ; 
						succSucc.pred = pred; 
						pred.succ = succSucc;
						succ.unlockSuccLock();
						pred.unlockSuccLock();
						removeFromTree(succ, successor, succParent);
						return succItem;
					}
				}
			}
			pred.unlockSuccLock();
		}
	}
	
	/**
	 * Acquire the treeLocks of the following nodes: 
	 * <ul>
	 * <li> The given node
	 * <li> The node's child - if the given node has less than two children
	 * <li> The node's successor, and the successor's parent and child - if the
	 * given node has two children
	 * </ul>
	 * 
	 * @param node The given node
	 * @return The node's successor, if the node has two children, and null,
	 * otherwise
	 */
	final private MapNode<K,V> acquireTreeLocks(final MapNode<K,V> node) {
		while (true) {
			node.lockTreeLock();
			final MapNode<K,V> right = node.right;
			final MapNode<K,V> left = node.left;
			if (right == null || left == null) {
				if (
					REBALANCE_MODE == RebalanceMode.Splay
					|| REBALANCE_MODE == RebalanceMode.None
				) {
					return null;
				}
				if (right != null && !right.tryLockTreeLock()) {
					node.unlockTreeLock();
					Thread.yield();
					continue;
				}
				if (left != null && !left.tryLockTreeLock()) {
					node.unlockTreeLock();
					Thread.yield();
					continue;
				}
				return null;
			}

			final MapNode<K,V> successor = node.succ;
			
			final MapNode<K, V> parent = successor.parent;
			if (parent != node) {
				if (!parent.tryLockTreeLock()) {
					node.unlockTreeLock();
					Thread.yield();
					continue;
				} else if (parent != successor.parent || !parent.valid) {
					parent.unlockTreeLock();
					node.unlockTreeLock();
					Thread.yield();
					continue;
				}
			}
			if (!successor.tryLockTreeLock()) { 
				node.unlockTreeLock();
				if (parent != node) parent.unlockTreeLock();
				Thread.yield();
				continue;
			}
			if (
				REBALANCE_MODE == RebalanceMode.Splay
				|| REBALANCE_MODE == RebalanceMode.None
			) {
				return successor;
			}
			final MapNode<K,V> succRightChild = successor.right; // there is no left child to the successor, perhaps there is a right one, which we need to lock.
			if (succRightChild != null && !succRightChild.tryLockTreeLock()) {
				node.unlockTreeLock();
				successor.unlockTreeLock();
				if (parent != node) parent.unlockTreeLock();
				Thread.yield();
				continue;
			}
			return successor;
		}
	}

	/**
	 * Removes the given node from the tree layout.
	 * If the node has less than two children, its successor, {@code succ}, is 
	 * null, and the removal is applied by connecting the node's parent to the 
	 * node's child. Otherwise, the successor is relocated to the node's location. 
	 * 
	 * @param node The node to remove
	 * @param succ The node's successor
	 * @param parent The node's parent
	 */
	private void removeFromTree(MapNode<K, V> node, MapNode<K, V> succ, 
			MapNode<K, V> parent) {
		if (succ == null) {
			MapNode<K, V> right = node.right;
			final MapNode<K,V> child = right == null ? node.left : right;
			boolean left = updateChild(parent, node, child);
			node.unlockTreeLock();
			parent.unlockTreeLock();
			return;
		}
		MapNode<K, V> oldParent = succ.parent;
		MapNode<K, V> oldRight = succ.right;
		updateChild(oldParent, succ, oldRight);

		MapNode<K, V> left = node.left;
		MapNode<K, V> right = node.right;
		succ.parent = parent;
		succ.left = left;
		succ.right = right; 
		left.parent = succ;
		if (right != null) {
			right.parent = succ;
		}
		if (parent.left == node) {
			parent.left = succ;
		} else {
			parent.right = succ;
		}
		boolean isLeft = oldParent != node;
		if (!isLeft) {
			oldParent = succ;
		} else {
			succ.unlockTreeLock();
		}
		node.unlockTreeLock();
		parent.unlockTreeLock();
		oldParent.unlockTreeLock();
	}

	/**
	 * Given a node, {@code parent}, its old child and a new child, update the
	 * old child with the new one.
	 * 
	 * @param parent The node
	 * @param oldChild The old child
	 * @param newChild The new child
	 * @return true if the old child was a left child  
	 */
	private boolean updateChild(MapNode<K, V> parent, MapNode<K, V> oldChild,
			final MapNode<K, V> newChild) {
		if (newChild != null) {
			newChild.parent = parent;
		}
		boolean left = parent.left == oldChild;
		if (left) {
			parent.left = newChild;
		} else {
			parent.right = newChild;
		}
		return left;
	}

	/**
	 * Splay the node.
	 * The splay is done by traversing the tree (starting from the given 
	 * node) and applying rotations. 
	 * 
	 * @param node The node to splay
	 */
	private void splay(MapNode<K,V> node, long depth) {
		long iterations = 0;
		if (ThreadLocalRandom.current().nextDouble() >= rotateProb(depth, iterations)) {
			return;
		}
		int conflicts = 0;
		node.lockTreeLock();
		if (!node.valid) {
			node.unlockTreeLock();
			return;
		}
		LockParentResult lockRes = tryLockParent(node, conflicts);
		if (lockRes.parent == null) {
			node.unlockTreeLock();
			return;
		}
		MapNode<K,V> parent = lockRes.parent;
		conflicts = lockRes.conflicts;
		while (parent != root) {
			lockRes = tryLockParent(parent, conflicts);
			if (lockRes.parent == null) {
				break;
			}
			MapNode<K,V> gParent = lockRes.parent;
			conflicts = lockRes.conflicts;
			if (gParent == root) {
				// zig
				rotate(node, parent, gParent, parent.left != node);
				parent.unlockTreeLock();
				parent = gParent;
				break;
			}
			lockRes = tryLockParent(gParent, conflicts);
			if (lockRes.parent == null) {
				gParent.unlockTreeLock();
				break;
			}
			MapNode<K,V> ggParent = lockRes.parent;
			if ((parent.left == node) == (gParent.left == parent)) {
				// zig-zig
				rotate(parent, gParent, ggParent, gParent.left != parent);
				rotate(node, parent, ggParent, parent.left != node);
			} else {
				// zig-zag
				rotate(node, parent, gParent, parent.left != node);
				rotate(node, gParent, ggParent, gParent.left != node);
			}
			depth -= 2;
			iterations++;
			parent.unlockTreeLock();
			gParent.unlockTreeLock();
			parent = ggParent;
			if (ThreadLocalRandom.current().nextDouble() >= rotateProb(depth, iterations)) {
				break;
			}
		}
		parent.unlockTreeLock();
		node.unlockTreeLock();
	}

	/**
	 * Apply a single rotation to the given node.
	 * 
	 * @param child The node's child
	 * @param node The node to rotate
	 * @param parent The node's parent
	 * @param left Is this a left rotation?
	 */
	final private void rotate(final MapNode<K,V> child, final MapNode<K,V> node, final MapNode<K,V> parent, boolean left) {
		if (STRUCT_MODS)
			counts.get().structMods += 1;
		if (parent.left == node) {
			parent.left = child;
		} else {
			parent.right = child;
		}
		child.parent = parent;
		node.parent = child;
		MapNode<K, V> grandChild = left? child.left : child.right;
		if (left) {
			node.right = grandChild;
			if (grandChild != null) {
				grandChild.parent = node; 
			}
			child.left = node;
		} else {
			node.left = grandChild;
			if (grandChild != null) {
				grandChild.parent = node; 
			}
			child.right = node;
		}
	}
	
	/**
	 * @see java.util.AbstractMap#clear()
	 */
	@Override
	public void clear() {
		root.parent.lockSuccLock();
		root.lockTreeLock();
		root.parent.succ = root;
		root.pred = root.parent;
		root.left = null;
		root.parent.unlockSuccLock();
		root.unlockTreeLock();
	}

	/**
	 * @return The height of the tree
	 */
	final public int height() {
		return height(root.left);
	}

	/**
	 * Returns the height of the sub-tree rooted at the given node.
	 * 
	 * @param node The given node
	 * @return The height of the sub-tree rooted by node
	 */
	final public int height(MapNode<K,V> node) {
		if (node == null) return 0;
		int rMax = height(node.right);
		int lMax = height(node.left);
		return Math.max(rMax, lMax) + 1;
	}

	/**
	 * @see java.util.AbstractMap#size()
	 */
	@Override
	final public int size() {
		return size(root.left);
	}

	/**
	 * Returns the number of nodes in the sub-tree rooted at the given node.
	 * 
	 * @param node The given node
	 * @return The number of nodes in the sub-tree rooted at node
	 */
	final public int size(MapNode<K,V> node) {
		if (node == null) return 0;
		int rMax = size(node.right);
		int lMax = size(node.left);
		return rMax+lMax + 1;
	}
	
	/**
	 * The tree is empty if the root's left child is empty
	 * @see java.util.AbstractMap#isEmpty()
	 */
	@Override
	public boolean isEmpty() {
		return root.left == null;
	}
	
	/**
	 * @see java.util.Map#entrySet()
	 */
	@Override
	public Set<java.util.Map.Entry<K, V>> entrySet() {
		return new AbstractSet<Map.Entry<K,V>>() {

			/**
			 * @see java.util.AbstractCollection#size()
			 */
			@Override
			public int size() {
				return LogicalOrderingSplay.this.size();
			}

			/**
			 * @see java.util.AbstractCollection#isEmpty()
			 */
			@Override
			public boolean isEmpty() {
				return LogicalOrderingSplay.this.isEmpty();
			}

			/**
			 * @see java.util.AbstractCollection#contains(java.lang.Object)
			 */
			@Override
			public boolean contains(Object o) {
				K key = (K) ((Entry) o).getKey();
				if (((Entry) o).getValue() == null) return false;
				V v = get(key);
				if (v == null) return false;
				return v.equals(((Entry) o).getValue());
			}
			
			/**
			 * @see java.util.AbstractCollection#add(java.lang.Object)
			 */
			@Override
			public boolean add(java.util.Map.Entry<K, V> e) {
				return put(e.getKey(), e.getValue()) != e.getValue();
			}
			
			/**
			 * @see java.util.AbstractCollection#remove(java.lang.Object)
			 */
			@Override
			public boolean remove(Object o) {
				return LogicalOrderingSplay.this.remove(((Entry) o).getKey(), ((Entry) o).getValue());
			}
			
			/**
			 * @see java.util.AbstractCollection#iterator()
			 */
			@Override
			public Iterator<java.util.Map.Entry<K, V>> iterator() {
				return new Iterator<Map.Entry<K,V>>() {
					
					private MapNode<K, V> curr = root.parent;
					private MapNode<K, V> currNext = curr;
					
					@Override
					public boolean hasNext() {
						getNext();
						return currNext != root;
					}

					@Override
					public java.util.Map.Entry<K, V> next() {
						getNext();
						curr = currNext;
						return curr == root? null : new SimpleImmutableEntry<K, V>(curr.key, (V) curr.item);
					}

					private void getNext() {
						if (currNext == curr) {
							currNext = curr.succ;
							while (!currNext.valid) currNext = curr.succ;
						}
					}

					@Override
					public void remove() {
						if (curr != root && curr != root.parent)
						LogicalOrderingSplay.this.remove(curr.key, curr.item);
					}
					
				};
			}
			
		};
	}
	
	/**
	 * A tree node
	 * 
	 * @author Dana
	 *
	 * @param <K>
	 * @param <V>
	 */
	class MapNode<K,V> {

		/** The node's key. */
		public final K key;
		
		/** The node's item. */
		public volatile Object item;
		
		/** Is the node valid? i.e. it was not marked as removed. */
		public volatile boolean valid;
		
		/** The predecessor of the node (with respect to the ordering layout). */
		public volatile MapNode<K, V> pred;
		
		/** The successor of the node (with respect to the ordering layout). */
		public volatile MapNode<K, V> succ;
		
		/** The lock that protects the node's {@code succ} field and the {@code pred} field of the node pointed by {@code succ}. */
		final public Lock succLock;

		/** The parent of the node (with respect to the tree layout). */
		public volatile MapNode<K, V> parent;
		
		/** The left child of the node (with respect to the tree layout). */
		public volatile MapNode<K, V> left;
		
		/** The right child of the node (with respect to the tree layout). */
		public volatile MapNode<K, V> right;

		/** The lock that protects the node's tree fields, that is, {@code parent, left, right, leftHeight, rightHeight}. */ 
		final public ReentrantLock treeLock;

		/**
		 * Constructor, create a new node.
		 * 
		 * @param key The new node's key
		 * @param item The new node's item
		 * @param pred The new node's predecessor (with respect to the ordering layout)
		 * @param succ The new node's successor (with respect to the ordering layout)
		 * @param parent The new node's parent (with respect to the tree layout)
		 */
		public MapNode(final K key, final Object item, final MapNode<K, V> pred, final MapNode<K, V> succ, final MapNode<K, V> parent) {
			this.key = key;
			this.item = item;
			valid = true;

			this.pred = pred;
			this.succ = succ;
			succLock = new ReentrantLock();
			
			this.parent = parent;
			right = null;
			left = null;
			treeLock = new ReentrantLock();
		}
		
		/**
		 * Constructor, create a new node with the given key.
		 *  
		 * @param key The new node's key
		 */
		public MapNode(K key) {
			this(key, null, null, null, null);
		}


		/**
		 * Lock the node's {@code treeLock}.
		 */
		public void lockTreeLock() {
			treeLock.lock();
		}

		/**
		 * Attempt to lock the node's {@code treeLock} without blocking.
		 * 
		 * @return true if the lock was acquired, and false otherwise
		 */
		public boolean tryLockTreeLock() {
			return treeLock.tryLock();
		}

		/**
		 * Release the node's {@code treeLock}.
		 */
		public void unlockTreeLock() {
			treeLock.unlock();
		}

		/**
		 * Lock the node's {@code succLock}.
		 */
		public void lockSuccLock() {
			succLock.lock();
		}

		/**
		 * Release the node's {@code succLock}.
		 */
		public void unlockSuccLock() {
			succLock.unlock();
		}

		/**
		 * @see java.lang.Object#toString()
		 */
		@Override
		public String toString() {
			String delimiter = "  ";
			StringBuilder sb = new StringBuilder();

			sb.append("(" + key + delimiter + ", " + valid + ")" + delimiter);

			return sb.toString();
		}
	}
}
