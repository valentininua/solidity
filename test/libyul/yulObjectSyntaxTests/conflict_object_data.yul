object "A" {
  code {}
  
  data "B" ""
  object "B" {
    code {}
  }
}
// ----
// ParserError 8794: (49-52): Object name "B" already exists inside the containing object.
