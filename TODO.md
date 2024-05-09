# NOTE

fir param: output length

# Optimization

+ ring buffer
+ fir
  + **NOTE**: Weights are flipped, so we need to change the weights accordingly 
  + Calling FIR on small arrays is bad. We should either:
    + Concatenate multiple rows and call at once
    + Call mac on the whole matrix (we need to pad zeros at the end of each kernel row when the input row size is not the same as the kernel row)

# Nice Chart

![Nice chart](figures/nice_chart.png)

# Paper

+ Cross functions opt - only possible from a top down view
    - API design perspective
      - OpenBLAS [FOR LOOP for Matrix Multiplication](https://github.com/OpenMathLib/OpenBLAS/issues/1636)
      - ONNX API design

# deal with stride

# optimize fc

# overflow

```
So, I think I figure out the overflow problem (thanks to help from Pouya). The issue is that the saturated computation might introduce errors to the intermediate results. For example, suppose I have a = 45k; b = 45k; c = -20k. With saturated addition of (a + b) + c, the result should be 17k (because a + b is saturated to 37k). However, the real results should be 70k (or 37k if the result is saturated).
I think this will happen to both in-layer computation or inter-layer computation. So, we proposed two solutions:
Use the scaled LEA computation, which will scale the intermediate results in LEA;
Use 32-bit for all intermediate results.
I think we should support all three of them (the current version, scaled LEA, and 32-bit intermediate results). They should have an increase accuracy/latency (showed by some plots). User can choose among any of these depending on the model and system constraints. (edited) 
2:33
Any comments?


Hank
  3:00 PM
I think all three of these are good.  My biggest concern is just not understanding exactly what problem they solve.  I understand these are solutions to overflow.  I am not sure where these come from.  Is this overflow that comes from the conversion of float to fixed (and thus independent of any compiler effort)?  If so, this is solving a broad problem and then we need to answer whether or not we are using the top-down approach to solve this problem as well.  Another possibilty is that this overflow is introduced by the top down approach (where it would otherwise be the users problem).  In that case, we need to address it as a challenge that comes from the top down approach (and woudl otherwise be forced on users).
3:02
So, I get the problem.  I like that you can support all three versions.  I am just thinking about how to present it in the paper. Does that make sense?


Mingyuan Xiang
  3:17 PM
Yes. I think is a general issue. Pouya told me he bypassed the issue by scaled LEA commands. Nevertheless we could also argue solving inter-layer saturation problem requires top-down knowledge.
3:17
Another reason for this is not a big issue for sonic is that they never had this serious oveflow
3:17
I think this is because I have a very weird training process (for quantization)


Hank
  3:18 PM
I like the second part.  If we can claim we solve it where users would otherwise have to address this themselves, that is a good thing to highlight as another opportunity of the top down approach applied to this substrate.
```

# Intermittent support
