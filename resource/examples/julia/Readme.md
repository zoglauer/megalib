# MEGAlib & Julia

## Setting up Julia

Install Julia from sources as stated here:
https://julialang.org/downloads/

Then:
```
export PYTHON=`which python3`
julia
> import Pkg
> Pkg.build("PyCall")
> exit()
```

## Run something:

```
julia SimParser.jl
```
 


