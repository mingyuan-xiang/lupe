"""Convert a pytorch model to onnx format"""

import argparse
import os
import importlib
import inspect

import torch
import torch.onnx

model_path = 'models_inference'
model_names = sorted([
    os.path.splitext(f)[0] for f in os.listdir(model_path) if f.endswith('.py')
])
models = {}

for m in model_names:
    module = importlib.import_module(model_path + '.' + m)

    for name, obj in inspect.getmembers(module):
        if inspect.isclass(obj):
            if obj.__module__ == module.__name__:
                models[m] = obj

nn_datasets = ['mnist', 'cifar10', 'vww', 'speech_commands', 'fashion_mnist']

parser = argparse.ArgumentParser(
    description='Convert a pytorch model to onnx format'
)

parser.add_argument('--dataset', default='mnist', choices=nn_datasets,
                    help='dataset (default: mnist)')
parser.add_argument('-a', '--arch', metavar='ARCH', default='LeNet',
                    choices=model_names,
                    help='model architecture: ' +
                        ' | '.join(model_names) +
                        ' (default: LeNet)')
parser.add_argument(
    '--param-loc', '-p',
    help='weights location (default: ./checkpoints/LeNet/model_best.pth)')
parser.add_argument('--output-loc', '-o', default='./onnx/',
                    help='output location (default: ./onnx/)')

args = parser.parse_args()

if args.param_loc is not None:
    param_loc = args.param_loc
else:
    param_loc = f'./checkpoints/{args.arch}/model_best.pth'

model = models[args.arch]()

print(model)

model.load_state_dict(torch.load(param_loc, map_location=torch.device('cpu'))['state_dict'])

if args.dataset == 'mnist':
    dummy_input = torch.randn(1, 1, 28, 28)
elif args.dataset == 'cifar10':
    dummy_input = torch.randn(1, 3, 32, 32)
elif args.dataset == 'vww':
    dummy_input = torch.randn(1, 3, 80, 80)
elif args.dataset == 'speech_commands':
    dummy_input = torch.randn(1, 1, 49, 12)
elif args.dataset == 'fashion_mnist':
    dummy_input = torch.randn(1, 1, 28, 28)
else:
    raise ValueError('Invalid dataset')

if not os.path.exists(args.output_loc):
    os.makedirs(args.output_loc)

torch.onnx.export(model, dummy_input,
    os.path.join(args.output_loc, args.arch + '.onnx'),
    input_names=["input"],
    output_names=["output"]
)
