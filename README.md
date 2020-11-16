# 一个功能简单的五子棋
> 作者：Kiprey

> E-mail: Kiprey@qq.com

## 简介
这个游戏是我在上手Qt时做的第一个项目

因为我在学Qt时直接上项目，所以代码可能会有点混乱

AI也是我的第一次尝试，所以肯定会存在代码长但不太智能的现象

该游戏支持AI对战与联机对战，同时还支持悔棋操作。

下载链接 - [release - 2.0](https://github.com/Kiprey/FiveChessGame/releases/tag/2.0)

## 后记

对于这个程序，代码有点混乱，算是不足的地方了

至于AI......

还算可以，可以和作者班级里（高二）五子棋最厉害的人打个平手

但是...优化是其中的不足之处，毕竟头一次做AI，就算想优化都不知道要从哪着手

但性能也克扣过于厉害...

本程序将不再进行更新，是时候朝下一个方向努力了！

留于 2018.04.14 周六 19:55

## 补充

> 该补充留于2020/11/2

三年后回顾这份代码，发现简直不能看QwQ

高耦合，混乱的类，杂乱的代码，简直就是一份shit mountain，而且就算想补救也只能重构代码了。

回顾了一下AI代码，终于发现效率低的原因了，AI算法的时间复杂度竟然是O(n^7)的......当场去世

这份代码的唯一目的也就是学习一下Qt的基本操作与概念了，至于其他的代码，不具有参考价值。
