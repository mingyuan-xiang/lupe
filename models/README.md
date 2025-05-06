## Converting PyTorch Model to ONNX

The script `to_onnx.py` do everything for you. For example:

```
python to_onnx.py --dataset cifar10 -a <model name> --param-loc checkpoints/<model name>/model_best.pth
```

This will convert a PyTorch mode