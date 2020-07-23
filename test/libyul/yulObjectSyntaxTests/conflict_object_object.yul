object "A" {
  code {}
  
  object "B" {
    code {}
  }
  object "B" {
    code {}
  }
}
// ----
// ParserError 8794: (66-69): Object name "B" already exists inside the containing object.
