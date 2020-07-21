{
	{ let d, e, i := f(1, 2, 3) }
	function f(a, b, c) -> x, y, z
	{
	   y := mload(a)
	   z := mload(c)
	}
}
// ----
// step: unusedFunctionParameterPruner
//
// {
//     {
//         let _3 := 3
//         let _4 := 2
//         let _5 := 1
//         let d, e, i := f_6(_5, _4, _3)
//     }
//     function f(a, c)
//     {
//         let y, z
//         let y_1 := mload(a)
//         y := y_1
//         let z_2 := mload(c)
//         z := z_2
//     }
//     function f_6(a, b, c) -> x, y, z
//     { f(a, c) }
// }
