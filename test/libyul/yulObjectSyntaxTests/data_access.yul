object "A" {
  code {
    pop(dataoffset("B"))
    pop(datasize("B"))
  }

  data "B" hex"00"
}
// ----
// TypeError 3517: (30-40): Unknown data object "B".
// TypeError 3517: (55-63): Unknown data object "B".
