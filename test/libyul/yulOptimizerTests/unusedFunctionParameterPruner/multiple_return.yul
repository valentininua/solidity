{
	// a, b, c are unused
	let a, b, c := f(sload(0))
	sstore(0, b)

	function f(d) -> x, y, z
	{
	   y := mload(1)
	   z := mload(2)
	}
}
// ----
// step: unusedFunctionParameterPruner
//
// {
//     let _3 := 0
//     let _4 := sload(_3)
//     let a, b, c := f_8(_4)
//     let _5 := 0
//     sstore(_5, b)
//     function f() -> y
//     {
//         let z
//         let _6 := 1
//         let y_1 := mload(_6)
//         y := y_1
//         let _7 := 2
//         let z_2 := mload(_7)
//         z := z_2
//     }
//     function f_8(d) -> x, y, z
//     { y := f() }
// }
