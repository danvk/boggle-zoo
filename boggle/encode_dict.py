#!/usr/bin/env python
import time
from collections import deque
from typing import Iterable, Self

LETTER_A = ord("a")


def is_boggle_word(word: str):
    size = len(word)
    if size < 3:
        return False
    for i, let in enumerate(word):
        if let < "a" or let > "z":
            return False
        if let == "q" and (i + 1 >= size or word[i + 1] != "u"):
            return False
    return True


def bogglify_word(word: str) -> str | None:
    if not is_boggle_word(word):
        return None
    return word.replace("qu", "q")


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
    words_under_: int
    children_: list[int]

    def __init__(self, is_word: bool, child_mask: int, words_under: int):
        self.is_word_ = is_word
        self.child_mask_ = child_mask
        self.children_ = []
        self.words_under_ = words_under

    def descend(self, nodes: list[Self], letter_idx: int) -> Self:
        assert 0 <= letter_idx < 26
        if not self.child_mask_ & (1 << letter_idx):
            return None
        child_idx = (self.child_mask_ & ((1 << letter_idx) - 1)).bit_count()
        return nodes[self.children_[child_idx]]

    def get_children(self, nodes: list[Self]):
        return [nodes[child] for child in self.children_]

    def count_words(self, nodes: list[Self]):
        num = 1 if self.is_word_ else 0
        for child in self.get_children(nodes):
            num += child.count_words(nodes)
        return num


def compact_trie(root: Node) -> list[CompactNode]:
    # Do two passes: one to number the nodes, a second to fill in those numbers.

    # Pass 1: number the (unique) nodes.
    node_ids = dict[Node, int]()
    q: deque[Node] = deque()
    q.append(root)
    while q:
        node = q.popleft()
        if node in node_ids:
            continue  # already been here
        node_ids[node] = len(node_ids)
        for child in node.children_.values():
            q.append(child)

    # Pass 2: fill in the compact nodes array.
    nodes: list[CompactNode] = [None] * len(node_ids)
    q.append(root)
    while q:
        node = q.popleft()
        id = node_ids[node]
        if nodes[id]:
            continue
        compact_node = CompactNode(node.is_word_, 0, node.words_under_)
        nodes[id] = compact_node
        compact_node.children_ = [node_ids[child] for child in node.children_.values()]

        for c, child in node.children_.items():
            i = ord(c) - LETTER_A
            compact_node.child_mask_ |= 1 << i
            q.append(child)

    # Make sure we've converted everything.
    max_delta = 0
    min_delta = 0
    for i, node in enumerate(nodes):
        for child in node.children_:
            delta = child - i
            min_delta = min(min_delta, delta)
            max_delta = max(max_delta, delta)
        assert node

    print(f"{min_delta=}, {max_delta=}")

    return nodes


def build_trie(words: Iterable[str]):
    root = Node()
    for word in words:
        root.add_word(word)
    return root


def write_binary_dict(nodes: list[CompactNode], output_file: str):
    """Write CompactNode list to binary file for C++ mmap."""

    import struct

    # Pass 1: construct an (implicit) array of nodes with gaps for children.
    n = 0
    node_to_index = dict[CompactNode, int]()
    for node in nodes:
        node_to_index[node] = n
        n += 2
        n += len(node.children_)
    num_slots = n

    # Pass 2: fill in the bytes
    n = 0
    min_delta = max_delta = 0
    with open(output_file, "wb") as f:
        for node in nodes:
            node_idx = n
            child_mask = node.child_mask_
            if node.is_word_:
                child_mask += 1 << 31
            f.write(struct.pack("I", child_mask))  # I = 32-bit unsigned
            f.write(struct.pack("I", node.words_under_))
            n += 2
            for child in node.get_children(nodes):
                child_idx = node_to_index[child]
                delta = child_idx - node_idx
                min_delta = min(delta, min_delta)
                max_delta = max(delta, max_delta)
                f.write(struct.pack("i", delta))  # i = 32-bit signed
                n += 1

    assert n == num_slots

    print(
        f"Wrote {num_slots} words to {output_file} ({num_slots * 4} bytes); {min_delta=} {max_delta=}"
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
    trie.count_words()
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


if __name__ == "__main__":
    main()
