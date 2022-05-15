# RegexEvaluator
A Regex Pattern Matching Algorithm in C, that I designed and implemented from scratch. Everything in the file above was coded by me without APIs. I only used tools such as malloc and printf that are essential in C.

I did not even read any documentation or guides on what the existing algorithm was. My aim was to try and develop this algorithm as if it was something entirely new, which I have successfully achieved. I was able to thus learn problem solving and learnt how to design a large-scale project. This is part of a C-Interpreter project I am writing. This will be in the backbone of the lexer for the Interpreter, wherein I will be adding the following features:
1. Text extraction - add a method for data to be extracted. For example, if one enters int xyz = 5;, then I will make a function to extract ' xyz ' and ' 5 ' after which, I will add a new:
2. Process function - do some pre-processing on the text to make it more usable (remove the spaces)


There is only one minor bug in the program that I found:
1. If you do something like (a*)+ which is basically (a+) which does work, but the (a*)+ doesn't work.
