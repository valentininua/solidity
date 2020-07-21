{
	function f() -> x
	{
		x := 1
	}
	pop(f())
}
// ----
// step: unusedFunctionParameterPruner
//
// {
//     let _2 := f()
//     pop(_2)
//     function f() -> x
//     {
//         let x_1 := 1
//         x := x_1
//     }
// }
