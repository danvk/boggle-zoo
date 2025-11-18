from cpp_boggle import Trie

from boggle.trie import bogglify_word


def asc(char: str):
    assert len(char) == 1
    return ord(char) - ord("a")


def test_trie():
    t = Trie.create_from_wordlist(
        [
            "agriculture",
            "culture",
            "boggle",
            "tea",
            "sea",
            "teapot",
        ]
    )
    assert not t.is_word("")  # Empty string is not a word

    assert t.size() == 6
    assert t.is_word("agriculture")
    assert t.is_word("culture")
    assert t.is_word("boggle")
    assert t.is_word("tea")
    assert t.is_word("sea")
    assert t.is_word("teapot")

    assert not t.is_word("teap")
    assert not t.is_word("random")
    assert not t.is_word("cultur")

    # Test prefix checking
    assert t.has_prefix("t")
    assert t.has_prefix("te")
    assert t.has_prefix("tea")
    assert t.get_mark("tea") == 0
    t.mark_word("tea", 12345)
    assert t.get_mark("tea") == 12345

    # Test word IDs
    assert t.get_word_id("agriculture") >= 0
    assert t.get_word_id("boggle") >= 0


def test_bogglify_word():
    assert bogglify_word("quart") == "qart"
    assert bogglify_word("qi") is None
    assert bogglify_word("is") is None
    assert bogglify_word("boggle") == "boggle"
    assert bogglify_word("quinquennia") == "qinqennia"


def test_load_file():
    t = Trie.create_from_file("testdata/boggle-words-4.txt")
    assert not t.is_word("")  # Empty string is not a word

    assert t.is_word("wood")
    assert not t.is_word("woxd")
