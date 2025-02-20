import xml.etree.ElementTree as ET
import sys

def radian2degree(radian):
    return radian * 180 / 3.1415926

def child_count(node):
    count = 0
    for child in node:
        if child.tag == 'body':
            count += 1
    return count

def read_node(file, node, lod):
    global channels_count
    for child in node:
        if child.tag == 'body':
            # 读joint名称、位置和旋转轴
            name = ''
            joint_offset = ''
            joint_axis = ''
            for child_child in child:
                if child_child.tag == 'joint':
                    name = child_child.attrib['name']
                    joint_offset = child_child.attrib['pos']
                    joint_axis = child_child.attrib['axis']
                    break
            
            # 从joint的相对位置计算绝对位置
            pos = '0 0 0' if child.attrib.get('pos') is None else child.attrib['pos']
            xyz_offset = joint_offset.split(' ')
            xyz_pos = pos.split(' ')
            xyz = [float(xyz_offset[i]) + float(xyz_pos[i]) for i in range(3)]
            xyz_quater = [a / 4.0 for a in xyz]
            pos = ' '.join([str(x) for x in xyz])
            pos_quater = ' '.join([str(x) for x in xyz_quater])

            # 判断自由度是x还是y还是z
            axis_str = joint_axis.split(' ')
            if float(axis_str[1]) != 0:
                axis = 2
                if float(axis_str[1]) < 0:
                    signs.append(-1)
                else:
                    signs.append(1)
            elif float(axis_str[2]) != 0:
                axis = 3
                if float(axis_str[2]) < 0:
                    signs.append(-1)
                else:
                    signs.append(1)
            else:
                axis = 1
                if float(axis_str[0]) < 0:
                    signs.append(-1)
                else:
                    signs.append(1)
            channels.append(axis)

            file.write('\t' * lod + 'JOINT ' + name + '\n')
            file.write('\t' * lod + '{\n')
            file.write('\t' * lod + '\tOFFSET ' + pos + '\n')
            file.write('\t' * lod + f'\tCHANNELS 3 Zrotation Xrotation Yrotation\n')
            channels_count += 3

            # 是否是最后一个节点
            cc = child_count(child)
            if cc == 0:
                file.write('\t' * (lod + 1) + 'End Site\n')
                file.write('\t' * (lod + 1) + '{\n')
                file.write('\t' * (lod + 1) + f'\tOFFSET {pos_quater}\n') # 用父节点的pos代替
                file.write('\t' * (lod + 1) + '}\n')
            else:
                read_node(file, child, lod + 1)                
            
            file.write('\t' * lod + '}\n')

if __name__ == '__main__':
    channels_count = 0
    channels_str = ['Xrotation', 'Yrotation', 'Zrotation']
    channels = []
    signs = []

    with open('./robot.bvh', 'w') as file:
        file.write('HIERARCHY\n')
        if len(sys.argv) != 5:
            print('Usage: python bvh_creator.py <input_file>')
            sys.exit(1)
        xml_file = sys.argv[1]
        anim_file = sys.argv[2]
        frame_count = sys.argv[3]
        frame_rate = sys.argv[4]

        # 解析mjcf文件的机器人结构
        tree = ET.parse(xml_file)
        root = tree.getroot()
        root_pos = ''
        for child in root:
            if child.tag == 'worldbody':
                body_root = child[0]
                root_pos = body_root.attrib['pos']
                file.write('ROOT ' + body_root.attrib['name'] + '\n')
                file.write('{\n')
                file.write('\tOFFSET ' + root_pos + '\n')
                file.write('\tCHANNELS 6 Xposition Yposition Zposition Zrotation Xrotation Yrotation\n')
                channels_count += 6
                read_node(file, body_root, 1)
                file.write('}\n')

        print(f'channels: {channels_count}')

        # file.write('MOTION\n')
        # file.write(f'Frames: {1}\n')
        # file.write(f'Frame Time: {0.0166667}\n')
        # dofs = [0] * channels_count
        # dofs[1] = 1.1
        # dofs[4] = -90 # 左右手系矫正
        # file.write(' '.join([str(x) for x in dofs]) + '\n')

        # 写入动画数据：必须保证数据的顺序与与采取的骨骼顺序完全一致
        file.write('MOTION\n')
        file.write(f'Frames: {frame_count}\n')
        file.write(f'Frame Time: {1.0 / int(frame_rate)}\n')
        print(len(signs), len(channels))
        with open(anim_file, 'r') as anim:
            for line in anim:
                frame = [0, 1.1, 0, 0, -90, 0]
                nums = [float(x) for x in line[:-2].split(' ')]
                for i, num in enumerate(nums):
                    frame.extend([0] * 3)
                    ang = radian2degree(num) * signs[i]
                    if channels[i] == 1:
                        frame[-2] = ang
                    elif channels[i] == 2:
                        frame[-1] = ang
                    else:
                        frame[-3] = ang
                file.write(' '.join([str(x) for x in frame]) + '\n')
