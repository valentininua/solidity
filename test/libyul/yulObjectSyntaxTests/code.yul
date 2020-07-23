object "A" {
  code {
    function f() -> offset, len {
      offset := dataoffset("A")
      len := datasize("A")
    }

    let offset, len := f()
    codecopy(0, offset, len)
  }

  data "hello" "hello world text"
  object "hello" {
    code {}
  }
}
// ----
// ParserError 8794: (226-233): Object name "hello" already exists inside the containing object.
// TypeError 3517: (72-82): Unknown data object "A".
// TypeError 3517: (101-109): Unknown data object "A".
