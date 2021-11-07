# CEGARBoxCPP
## Dependencies
This project require minisat installed to run and compile. Instruction for installing minisat are available [here](https://github.com/niklasso/minisat).

## Compile
Run ``./compile.sh`` to compile CEGARBox

## Input Formula
CEGARBox accepts file input. Input is terminated by a newline and valid input formula are defined by the following grammar:
```
Formula := ImpFormula <=> ImpFormula || ImpFormula
ImpFormula := OrFormula => OrFormula || OrFormula
OrFormula := AndFormula | AndFormula || AndFormula
AndFormula := FormulaRest & FormulaRest || FormulaRest
FormulaRest := (Formula) || ~FormulaRest || [Int]FormulaRest || <Int>FormulaRest || $true || $false || Atom
Atom := Alphanumeric String
```

## Run Theorem Prover

``./main -f <input_file> [options]``

Options:

* Reflexivity: ``--reflexive`` or ``-t``
* Symmetry: ``--symmetric`` or ``-b``
* Transitivity: ``--transitive`` or ``-4``
* Seriality: ``--serial`` or ``-d``
* Euclidean: ``--euclidean`` or ``-e``
* Valid (whether input formula is valid): ``--valid`` or ``-v``