import argparse
import math

parser = argparse.ArgumentParser(
    description="")
parser.add_argument(
    "r",
    type=float)
parser.add_argument(
    "g",
    type=float)
parser.add_argument(
    "b",
    type=float)
args = parser.parse_args()

def clamp(x):
    return min(1, max(0, x))

# l = (args.r + args.g + args.b)/math.sqrt(3)
# s = math.sqrt(max(args.r**2 + args.g**2 + args.b**2 - l**2, 0))
#
# lr = (1 + 0 + 0)/math.sqrt(3) # hue is relative to RED (1,0,0) direction
# hr = (1-lr/math.sqrt(3), 0-lr/math.sqrt(3), 0-lr/math.sqrt(3))
#
# hv = (args.r-l/math.sqrt(3), args.g-l/math.sqrt(3), args.b-l/math.sqrt(3))
# abs_hv = math.sqrt(hv[0]**2 + hv[1]**2 + hv[2]**2)
# abs_hv = abs_hv if abs_hv > 0 else 1 # just in case l == 0
# abs_hr = math.sqrt(hr[0]**2 + hr[1]**2 + hr[2]**2)
# cos_h = (hv[0]*hr[0] + hv[1]*hr[1] + hv[2]*hr[2])/abs_hv/abs_hr
# h = math.acos(min(1, max(-1, cos_h)))
#
# # used to determine direction of angle
# hd = (hr[0]-hv[0], hr[1]-hv[1], hr[2]-hv[2])
# if hd[1] >= 0.0 and hd[2] < 0.0:
#     h += math.pi

# Y then X
theta_y = -math.pi/4
cy = math.cos(theta_y)
sy = math.sin(theta_y)
theta_x = -math.pi/2 + math.atan2(math.sqrt(2),1)
cx = math.cos(theta_x)
sx = math.sin(theta_x)

hx = cy*args.r + sy*args.b
hy = -sx*sy*args.r + cx*args.g + sx*cy*args.b
hz = -cx*sy*args.r - sx*args.g + cx*cy*args.b
h = math.atan2(hy, hx)
s = math.sqrt(hx**2 + hy**2)
l = hz

# if h < 0:
#     h += math.pi

# print("hx:{0} hy:{1} hz:{2}".format(hx, hy, hz))
# print("h:{0} s:{1} l:{2}".format(h, s, l))
print("h:{0} s:{1} l:{2}".format(360*h/(2*math.pi), s/(math.sqrt(3)/2), l/math.sqrt(3)))

# https://en.wikipedia.org/wiki/Transformation_matrix#Rotation_2
x = s*math.cos(h)
y = s*math.sin(h)
z = l

# X then Y
theta_x = math.pi/2 - math.atan2(math.sqrt(2),1)
cx = math.cos(theta_x)
sx = math.sin(theta_x)
theta_y = math.pi/4
cy = math.cos(theta_y)
sy = math.sin(theta_y)

xx = cy*x - sy*sx*y + sy*cx*z
yy = cx*y + sx*z
zz = -sy*x - cy*sx*y + cy*cx*z

# print("x:{0} y:{1} z:{2}".format(x, y, z))
print("r:{0} g:{1} b:{2}".format(xx, yy, zz))
# print("r:{0} g:{1} b:{2}".format(clamp(xx), clamp(yy), clamp(zz)))
