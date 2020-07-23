object "A" {
  code {}
  
  data "B" ""
  data "B" hex"00"
}
// ----
// ParserError 8794: (47-50): Object name "B" already exists inside the containing object.
