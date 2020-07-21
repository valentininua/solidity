{
    function f(a, b)
    {
        a := mload(a)
        a := add(a, 1)
    }
}
// ----
// step: unusedFunctionParameterPruner
//
// {
//     function f(a)
//     {
//         let a_3 := a
//         let a_1 := mload(a_3)
//         a := a_1
//         let _4 := 1
//         let a_2 := add(a_1, _4)
//         a := a_2
//     }
//     function f_5(a, b)
//     { f(a) }
// }
