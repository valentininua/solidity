{
    let a, b := f(mload(0), mload(1))
    sstore(0, a)

    function f(c, d) -> x, y
    {
            x := add(c, d)
            y := add(c, d)
    }
}
// ----
// step: unusedFunctionParameterPruner
//
// {
//     let _3 := 1
//     let _4 := mload(_3)
//     let _5 := 0
//     let _6 := mload(_5)
//     let a, b := f_8(_6, _4)
//     let _7 := 0
//     sstore(_7, a)
//     function f(c, d) -> x
//     {
//         let y
//         let x_1 := add(c, d)
//         x := x_1
//         let y_2 := add(c, d)
//         y := y_2
//     }
//     function f_8(c, d) -> x, y
//     { x := f(c, d) }
// }
