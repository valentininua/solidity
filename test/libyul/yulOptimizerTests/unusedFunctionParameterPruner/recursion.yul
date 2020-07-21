{
    function f(a, b, c) -> x, y, z
    {
        x, y, z := f(1, 2, 3)
        x := add(x, 1)
    }
}
// ----
// step: unusedFunctionParameterPruner
//
// {
//     function f() -> x, y, z
//     {
//         let _5 := 3
//         let _6 := 2
//         let _7 := 1
//         let x_1, y_2, z_3 := f_9(_7, _6, _5)
//         x := x_1
//         y := y_2
//         z := z_3
//         let _8 := 1
//         let x_4 := add(x_1, _8)
//         x := x_4
//     }
//     function f_9(a, b, c) -> x, y, z
//     { x, y, z := f() }
// }
