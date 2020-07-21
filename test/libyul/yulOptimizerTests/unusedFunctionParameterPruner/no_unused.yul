{
	function f(a) -> x { x := a }
	function g(b) -> y { pop(g(b)) }
}
// ----
// step: unusedFunctionParameterPruner
//
// {
//     function f(a) -> x
//     {
//         let x_1 := a
//         x := x_1
//     }
//     function g(b)
//     {
//         let _2 := g_3(b)
//         pop(_2)
//     }
//     function g_3(b) -> y
//     { g(b) }
// }
