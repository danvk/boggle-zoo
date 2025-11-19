#!/usr/bin/env python
import time
from collections import deque
from typing import Iterable, Self

from boggle.trie import bogglify_word

LETTER_A = ord("a")


class Node:
    is_word_: bool
    children_: dict[str, "Node"]

    def __init__(self):
        self.is_word_ = False
        self.children_ = {}
        self.words_under_ = None
        self.tracking_ = None

    def __hash__(self):
        """Hash based on the node's structure for DAWG deduplication."""
        return hash(
            (
                self.is_word_,
                tuple(sorted((k, id(v)) for k, v in self.children_.items())),
            )
        )

    def __eq__(self, other):
        """Two nodes are equal if they have the same structure."""
        if not isinstance(other, Node):
            return False
        return (
            self.is_word_ == other.is_word_
            and self.children_.keys() == other.children_.keys()
            and all(
                self.children_[k] is other.children_[k] for k in self.children_.keys()
            )
        )

    def add_word(self, word: str):
        if not word:
            self.is_word_ = True
        else:
            first = word[0]
            rest = word[1:]
            if first not in self.children_:
                self.children_[first] = Node()
            self.children_[first].add_word(rest)

    def starts_word(self, letter_index: int) -> bool:
        return self.descend(letter_index) is not None

    def descend(self, letter_index: int) -> Self | None:
        return self.children_.get(chr(LETTER_A + letter_index))

    def count_words(self):
        if self.words_under_ is not None:
            return self.words_under_
        words_under = (1 if self.is_word_ else 0) + sum(
            c.count_words() for c in self.children_.values()
        )
        self.words_under_ = words_under
        return words_under

    def count_nodes(self, visited=None):
        """Count unique nodes in the trie/DAWG.

        Uses a visited set to avoid double-counting shared nodes in DAWGs.
        """
        if visited is None:
            visited = set()

        # If we've already visited this node, don't count it again
        if id(self) in visited:
            return 0

        visited.add(id(self))
        return 1 + sum(c.count_nodes(visited) for c in self.children_.values())

    def is_word(self, word: str):
        if not word:
            return self.is_word_
        first = word[0]
        rest = word[1:]
        child = self.children_.get(first)
        if not child:
            return False
        return child.is_word(rest)

    def set_tracking(self):
        if not self.words_under_:
            self.count_words()
        if not self.children_:
            return
        child_counts = [c.words_under_ for c in self.children_.values()]
        partial_sum = 0
        partial_sums = [0]
        for count in child_counts[:-1]:
            partial_sum += count
            partial_sums.append(partial_sum)
        for child, count in zip(self.children_.values(), partial_sums):
            child.tracking_ = count
        for child in self.children_.values():
            child.set_tracking()

    def get_word_index(self, word: str):
        assert self.words_under_ is not None
        if not word:
            assert self.is_word_
            return 0
        idx = 1 if self.is_word_ else 0
        first = word[0]
        rest = word[1:]
        for letter, child in self.children_.items():
            if letter < first:
                idx += child.words_under_
            elif letter == first:
                idx += child.get_word_index(rest)
        return idx

    def get_word_index_tracking(self, word: str):
        if not word:
            assert self.is_word_
            return 0

        first = word[0]
        rest = word[1:]
        child = self.children_[first]
        # first_child = next(iter(self.children_.values()))
        return (
            (1 if self.is_word_ else 0)
            + child.tracking_
            # - first_child.tracking_
            + child.get_word_index_tracking(rest)
        )

    def to_dot(self):
        """Convert the trie to GraphViz DOT format.

        Returns a string containing the DOT specification.
        Nodes that represent complete words are shown as double circles.
        Edges are labeled with their letters.
        """
        lines = ["digraph Trie {"]
        lines.append("  rankdir=TB;")
        lines.append("  node [shape=circle];")

        self.count_words()  # populate words_under_

        # Use a counter to assign unique IDs to nodes
        node_counter = [0]
        node_ids = {}  # Map from node object id to DOT node id

        def get_node_id(node):
            node_key = id(node)
            if node_key not in node_ids:
                node_ids[node_key] = f"n{node_counter[0]}"
                node_counter[0] += 1
            return node_ids[node_key]

        visited = set()

        def traverse(node, parent_id=None):
            node_id = get_node_id(node)
            node_key = id(node)

            # If we've already visited this node, don't traverse it again
            if node_key in visited:
                return
            visited.add(node_key)

            # Create node with appropriate shape
            if node.is_word_:
                shape = "shape=doublecircle"
            else:
                shape = ""
            label = f'label="{node.tracking_}"'
            lines.append(f"  {node_id} [{shape} {label}];")

            # Create edges to children
            for letter, child in sorted(node.children_.items()):
                child_id = get_node_id(child)
                lines.append(f'  {node_id} -> {child_id} [label="{letter}"];')
                traverse(child, node_id)

        traverse(self)
        lines.append("}")
        return "\n".join(lines)

    def to_dawg(self):
        """Convert the trie to a DAWG (Directed Acyclic Word Graph).

        This minimizes the trie by merging nodes with identical suffixes.
        Returns the root node of the DAWG.
        """
        # Registry of unique nodes we've seen
        registry = {}

        def minimize(node):
            """Recursively minimize the trie from bottom-up."""
            # First, minimize all children
            for letter in list(node.children_.keys()):
                node.children_[letter] = minimize(node.children_[letter])

            # Check if we've seen an equivalent node before
            if node in registry:
                # Return the existing equivalent node
                return registry[node]
            else:
                # Register this node as unique
                registry[node] = node
                return node

        return minimize(self)


class CompactNode:
    is_word_: bool
    child_mask_: int
    first_child_: int
    tracking_: int

    def __init__(self, is_word: bool, child_mask: int, first_child: int, tracking: int):
        self.is_word_ = is_word
        self.child_mask_ = child_mask
        self.first_child_ = first_child
        self.tracking_ = tracking

    def descend(self, nodes: list[Self], letter_idx: int) -> Self:
        assert 0 <= letter_idx < 26
        if not self.child_mask_ & (1 << letter_idx):
            return None
        child_idx = (self.child_mask_ & ((1 << letter_idx) - 1)).bit_count()
        return nodes[self.first_child_ + child_idx]

    def get_children(self, nodes: list[Self]):
        child = self.first_child_
        child_mask = self.child_mask_
        while child_mask:
            while child_mask & 1 == 0:
                child_mask >>= 1
            yield nodes[child]
            child += 1
            child_mask >>= 1

    def count_words(self, nodes: list[Self]):
        num = 1 if self.is_word_ else 0
        for child in self.get_children(nodes):
            num += child.count_words(nodes)
        return num

    def is_word(self, nodes: list[Self], word: str) -> bool:
        if not word:
            return self.is_word_
        first = word[0]
        rest = word[1:]
        idx = ord(first) - LETTER_A
        child = self.descend(nodes, idx)
        if not child:
            return False
        return child.is_word(nodes, rest)

    def get_word_index(self, nodes: list[Self], word: str) -> int:
        if not word:
            assert self.is_word_
            return 0

        first = word[0]
        rest = word[1:]
        child = self.descend(nodes, ord(first) - LETTER_A)
        return (
            (1 if self.is_word_ else 0)
            + child.tracking_
            + child.get_word_index(nodes, rest)
        )

    @staticmethod
    def _get_canonical_children(
        nodes: list["CompactNode"], node_idx: int, canonical_map: dict[int, int]
    ) -> list[int]:
        """Get the list of canonical child indices for a node."""
        node = nodes[node_idx]
        canon_children = []
        child_offset = 0
        for bit in range(26):
            if node.child_mask_ & (1 << bit):
                old_child_idx = node.first_child_ + child_offset
                canon_idx = canonical_map.get(old_child_idx, old_child_idx)
                canon_children.append(canon_idx)
                child_offset += 1
        return canon_children

    @staticmethod
    def _find_subsequence(haystack: list[int], needle: list[int]) -> int | None:
        """Find if needle appears as a consecutive subsequence in haystack.

        Returns the starting index if found, None otherwise.
        """
        if not needle:
            return None
        needle_len = len(needle)
        haystack_len = len(haystack)
        if needle_len > haystack_len:
            return None
        for i in range(haystack_len - needle_len + 1):
            if haystack[i : i + needle_len] == needle:
                return i
        return None

    @staticmethod
    def to_dawg(nodes: list["CompactNode"]) -> list["CompactNode"]:
        """Convert CompactNode list to DAWG by consolidating equivalent nodes.

        Returns a new list[CompactNode] where root is at index 0.
        Two nodes are equivalent if they have the same is_word_, child_mask_,
        tracking_, and all their children are equivalent.
        """
        if not nodes:
            return nodes

        # Step 1: Compute canonical mapping (bottom-up)
        # Maps each node index to its canonical representative
        canonical_map = {}
        signature_map = {}  # Maps signature to canonical index

        start_s = time.time()
        # Process nodes in reverse order (leaves first)
        for old_idx in range(len(nodes) - 1, -1, -1):
            node = nodes[old_idx]

            # Get canonical children for this node's signature
            canon_children = CompactNode._get_canonical_children(
                nodes, old_idx, canonical_map
            )

            # Create signature for this node
            sig = (
                node.is_word_,
                node.child_mask_,
                node.tracking_,
                tuple(canon_children),
            )

            if sig in signature_map:
                # This node is equivalent to an existing one
                canonical_map[old_idx] = signature_map[sig]
            else:
                # This node is canonical
                canonical_map[old_idx] = old_idx
                signature_map[sig] = old_idx

        part1_s = time.time()
        print(f"Part 1: {part1_s - start_s} s")

        # Step 2: BFS to build new_nodes, sharing children subsequences
        # Key invariant: children are at consecutive indices starting at first_child_
        # Optimization: share any subsequence of children, not just full sequences
        new_nodes = []
        new_nodes_canonical = []  # Tracks which old canonical idx each new node represents
        subseq_map = {}  # Maps any subsequence (tuple) -> first index where it appears
        queue = []

        # Statistics
        subseq_match_count = 0
        subseq_create_count = 0

        # Start with root's canonical node
        root_canon_idx = canonical_map[0]
        root_node = nodes[root_canon_idx]
        new_nodes.append(
            CompactNode(
                root_node.is_word_, root_node.child_mask_, 0, root_node.tracking_
            )
        )
        new_nodes_canonical.append(root_canon_idx)
        # Register single-node subsequence
        subseq_map[(root_canon_idx,)] = 0
        queue.append((root_canon_idx, 0))

        while queue:
            old_canon_idx, new_idx = queue.pop(0)

            if not nodes[old_canon_idx].child_mask_:
                continue

            # Get canonical children sequence
            canon_children = CompactNode._get_canonical_children(
                nodes, old_canon_idx, canonical_map
            )
            canon_children_tuple = tuple(canon_children)

            # Try subsequence lookup (fast O(1) hash lookup)
            first_child_idx = subseq_map.get(canon_children_tuple)

            if first_child_idx is not None:
                # Subsequence match found
                new_nodes[new_idx].first_child_ = first_child_idx
                subseq_match_count += 1
            else:
                # No match - create new sequence
                first_child_idx = len(new_nodes)
                new_nodes[new_idx].first_child_ = first_child_idx
                subseq_create_count += 1

                # Add all children consecutively and enqueue for processing
                start_idx = len(new_nodes)
                for canon_child_idx in canon_children:
                    child_node = nodes[canon_child_idx]
                    new_nodes.append(
                        CompactNode(
                            child_node.is_word_,
                            child_node.child_mask_,
                            0,  # Will be set when this node is processed
                            child_node.tracking_,
                        )
                    )
                    new_nodes_canonical.append(canon_child_idx)
                    queue.append((canon_child_idx, len(new_nodes) - 1))

                # Register all subsequences starting at start_idx
                # This allows any future sequence to match against any part of what we just added
                num_added = len(canon_children)
                for subseq_start in range(num_added):
                    for subseq_end in range(subseq_start + 1, num_added + 1):
                        subseq = tuple(canon_children[subseq_start:subseq_end])
                        # Only register if not already present (keep first occurrence)
                        if subseq not in subseq_map:
                            subseq_map[subseq] = start_idx + subseq_start

        part2_s = time.time()
        print(f"Part 2: {part2_s - part1_s} s")
        print(
            f"Sharing stats: {subseq_match_count} matched, {subseq_create_count} created, {len(subseq_map)} subsequences in map"
        )
        return new_nodes


def compact_trie(root: Node) -> list[CompactNode]:
    nodes: list[CompactNode] = []
    # (current node, parent node index, child index)
    q: deque[tuple[Node, int, int]] = deque()
    q.append((root, None, -1))
    max_offset = 0
    while q:
        node, parent_idx, child_index = q.popleft()
        if node == 0:
            continue

        compact_node_idx = len(nodes)
        compact_node = CompactNode(node.is_word_, 0, 0, node.tracking_)
        nodes.append(compact_node)

        # Record the first child offset when it's added
        if parent_idx is not None and child_index == 0:
            delta = compact_node_idx - parent_idx
            nodes[parent_idx].first_child_ = compact_node_idx
            max_offset = max(max_offset, delta)

        num_children = 0
        for c, child in node.children_.items():
            i = ord(c) - LETTER_A
            compact_node.child_mask_ |= 1 << i
            q.append((child, compact_node_idx, num_children))
            num_children += 1
    return nodes


def build_trie(words: Iterable[str]):
    root = Node()
    for word in words:
        root.add_word(word)
    return root


def write_binary_dict(nodes: list[CompactNode], output_file: str):
    """Write CompactNode list to binary file for C++ mmap.

    Binary format using bitfields (8 bytes per node):
    struct CompactNode {
      uint64_t child_mask : 26;   // Bitmask for which children exist
      uint64_t is_word : 1;       // 1 if this node represents a complete word
      uint64_t first_child : 21;  // Index of first child (-1 if no children)
      uint64_t mark : 16;         // Mark for tracking during searches
    };
    """
    import ctypes

    class CompactNodeBinary(ctypes.Structure):
        _fields_ = [
            ("padding", ctypes.c_uint32, 5),
            ("child_mask", ctypes.c_uint32, 26),
            ("is_word", ctypes.c_uint32, 1),
            ("first_child", ctypes.c_uint16, 16),
            ("mark", ctypes.c_uint16, 16),
        ]

    max_offset = 0
    with open(output_file, "wb") as f:
        for i, node in enumerate(nodes):
            child_offset = 0
            if node.child_mask_:
                child_offset = node.first_child_ - i
                assert 0 < child_offset < 2**16, child_offset
                max_offset = max(child_offset, max_offset)
            binary_node = CompactNodeBinary(
                child_mask=node.child_mask_ & 0x3FFFFFF,  # 26 bits
                is_word=1 if node.is_word_ else 0,  # 1 bit
                first_child=child_offset & 0x1FFFFF,  # 21 bits
                mark=0,  # 16 bits, initially 0
            )
            f.write(bytes(binary_node))

    print(
        f"Wrote {len(nodes)} nodes to {output_file} ({len(nodes) * 8} bytes); {max_offset=}"
    )


def main():
    import argparse

    parser = argparse.ArgumentParser(
        description="Build and analyze a trie from a word list"
    )
    parser.add_argument("input_file", help="Input file containing words (one per line)")
    parser.add_argument("--dot", action="store_true", help="Output GraphViz DOT format")
    parser.add_argument(
        "--dawg",
        action="store_true",
        help="Convert trie to DAWG (minimize by merging nodes with identical suffixes)",
    )
    parser.add_argument(
        "--check_index", action="store_true", help="Verify tracking index for all words"
    )
    parser.add_argument(
        "--binary",
        "-b",
        metavar="OUTPUT",
        help="Write binary dictionary file for C++ mmap",
    )

    args = parser.parse_args()

    words = [line.strip() for line in open(args.input_file)]
    # Filter and bogglify words (remove words < 3 chars, replace "qu" with "q")
    words = [bogglify_word(word) for word in words]
    words = [word for word in words if word is not None]
    words.sort()
    trie = build_trie(words)
    # trie.set_tracking()

    if args.dawg:
        trie = trie.to_dawg()

    if args.dot:
        print(trie.to_dot())
    else:
        structure_type = "DAWG" if args.dawg else "Trie"
        print(
            f"{args.input_file}: {trie.count_words()} words, {trie.count_nodes()} nodes ({structure_type})"
        )

    start_s = time.time()
    compact_nodes = compact_trie(trie)
    end_s = time.time()
    print(f"Compact trie: {end_s - start_s} s")
    # compact_dawg = CompactNode.to_dawg(compact_nodes)
    # print(f"Compact {len(compact_nodes)} -> {len(compact_dawg)}")

    if args.binary:
        write_binary_dict(compact_nodes, args.binary)

    if args.check_index:
        compact_root = compact_nodes[0]
        for idx, word in enumerate(words):
            assert trie.is_word(word)
            calc_idx = trie.get_word_index(word)
            calc_idx_tracking = trie.get_word_index_tracking(word)
            assert calc_idx == idx, f"{word} {calc_idx} != {idx}"
            assert calc_idx_tracking == idx, f"{word} {calc_idx_tracking} != {idx}"
            calc_idx_dawg = compact_root.get_word_index(compact_nodes, word)
            assert calc_idx_dawg == idx, f"{word} {calc_idx_dawg} != {idx}"


if __name__ == "__main__":
    main()
