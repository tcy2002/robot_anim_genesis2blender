## Genesis机器人动画导出、导入Blender笔记

### 背景

需要将Genesis的机器人模拟数据以通用格式导出，然后导入Blender以制作高质量渲染效果。Genesis目前不支持通用场景格式导出（.usd），必须手写导出逻辑，Blender不支持mjcf/urdf等机器人通用格式的导入，但支持bvh和gltf格式，因此genesis可以采用导出bvh或gltf的两种方案；bvh是骨骼动画，格式简单，只关注骨骼节点的位置和旋转信息，导入blender后需要额外的绑定工作，gltf包括完整mesh和动画数据，一劳永逸。机器人模型是宇树h1带手

## 方案1：导出bvh

### 思路

1、Genesis模拟，并导出每一帧每个自由度的旋转量

2、从mjcf格式中解析出机器人格式对应的HIERARCHY，包括节点父子关系，偏移量，自由度（旋转轴）等，生成bvh文件的框架

3、按照bvh要求的数据格式写入步骤1的数据到bvh中

4、导入blender，重新绑定mesh和mat到骨骼上

由于mjcf格式和bvh格式有一定区别，bvh无法获得末端节点的位置信息（即指尖、脚等）

### 格式匹配

bvh：
```
HIERARCHY
ROOT Hips
{
    OFFSET  0.00    0.00    0.00
    CHANNELS 6 Xposition Yposition Zposition Zrotation Xrotation Yrotation
    JOINT Chest
    {
        OFFSET   0.00    5.21    0.00
        CHANNELS 3 Zrotation Xrotation Yrotation
        JOINT Head
        {
            ...
        }
    }
    ...
}
MOTION
Frames: 470
Frame Time: 0.016666666666666666
0 1.1 0 0 -90 0 0.0 0 0 0 0.0 0 0 0 0.0 0 0 0.0 ...
...
```
mjcf：

```
<mujoco model="h1">
	<default>...</default>
	<asset>...</asset>
	<worldbody>
		<body>
			<joint>...</joint>
			<body>
				...
			</body>
			...
		</body>
		...
	</worldbody>
	<actuator>...</actuator>
	<sensor>...</sensor>
</mujoco>
```
mjcf主要关注worldbody部分，body代表link（bone），joint代表骨骼点，body的位置+joint的位置就是bvh里joint的OFFSET
mjcf里joint的旋转轴对应bvh的CHANNELS，bvh是角度，Genesis输出的是弧度

### Blender导入效果（未做骨骼绑定）

![bvh](bvh.gif)

## 方案2：导出gltf

之前有过导出gltf的经验，有代码参考（Physics-Engine）

### 思路

1、Genesis模拟，导出每一帧每个Link的位置和旋转量（position和quaternion）

2、从mjcf格式中解析出机器人模型对应的obj文件，以及每个mesh相对于link的位置

3、按照gltf要求的数据格式将步骤1的数据写入gltf

4、导入到blender

### mjcf的几何信息：


```
<geom class="visual" mesh="pelvis"/>
<geom class="collision" mesh="pelvis"/>
<geom type="mesh" contype="0" conaffinity="0" group="1" density="0" rgba="0.1 0.1 0.1 1" mesh="left_hand_link"/>
<geom type="mesh" rgba="0.1 0.1 0.1 1" mesh="left_hand_link"/>
<geom pos="0.003 0 0" quat="0.707107 0 0 0.707107" type="mesh" contype="0" conaffinity="0" group="1" density="0" rgba="0.1 0.1 0.1 1" mesh="L_hand_base_link"/>
<geom pos="0.003 0 0" quat="0.707107 0 0 0.707107" type="mesh" rgba="0.1 0.1 0.1 1" mesh="L_hand_base_link"/>
```
这是每个Body（Link）可能出现的几何配置，要选择合适的条目读取mesh

### 导入Blender效果

![gltf](gltf.gif)

### 文件解释

- humanoid.py：Genesis模拟脚本
- dofs.txt：Genesis导出的自由度信息
- bvh_creater.py：生成bvh的脚本
- robot.bvh：生成的bvh文件
- amim.txt：Genesis导出的位置和旋转信息
- mjcf_parser.py：读取mjcf几何信息的脚本
- link2mesh.txt：mjcf几何信息
- robot.gltf：生成的gltf文件
- gltf_writer：创建gltf的工具（from Physics-Engine）

