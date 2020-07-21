{
    let z := f(1)
    function f(x) -> y
	{
		let w := mload(1)
		y := mload(w)
	}
}
// ----
// step: unusedFunctionParameterPruner
//
// {
//     let _2 := 1
//     let z := f_4(_2)
//     function f()
//     {
//         let y
//         let _3 := 1
//         let w := mload(_3)
//         let y_1 := mload(w)
//         y := y_1
//     }
//     function f_4(x) -> y
//     { f() }
// }
