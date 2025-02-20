import numpy as np
import genesis as gs

########################## 初始化 ##########################
gs.init(backend=gs.gpu)

########################## 创建场景 ##########################
scene = gs.Scene(
    viewer_options = gs.options.ViewerOptions(
        camera_pos    = (0, -3.5, 2.5),
        camera_lookat = (0.0, 0.0, 0.5),
        camera_fov    = 30,
        max_FPS       = 60,
    ),
    sim_options = gs.options.SimOptions(
        dt = 0.01,
    ),
    show_viewer = True,
)

########################## 实体 ##########################
# 地面
plane = scene.add_entity(
    gs.morphs.Plane(),
)

# 机器人：宇树h1带手
robot = scene.add_entity(
    gs.morphs.MJCF(
        file  = 'xml/h1_description/mjcf/h1_with_hand.xml',
        pos   = (0.0, 0.0, 0.0),
        euler = (0, 0, 0),
    ),
)

# 目标物体
# ## 简单圆柱体
# cylinder = scene.add_entity(
#     gs.morphs.Cylinder(
#         height = 0.12,
#         radius = 0.03,
#         pos  = (0.67, 0.08, 0.06),
#     )
# )
# ## 有柄咖啡杯
# mesh = scene.add_entity(
#     morph = gs.morphs.Mesh(
#         file = "meshes/cups/nescafe/nescafe_mug.obj",
#         scale = 1,
#         pos = (0.67, 0.08, 0.06),
#         euler = (90, 0, 0),
#     ),
# )
# ## 无柄阔口杯
# mesh = scene.add_entity(
#     morph = gs.morphs.Mesh(
#         file = "meshes/cups/old_wooden_cup/WoodenCup.obj",
#         scale = 0.038,
#         pos = (0.67, 0.08, 0.06),
#         euler = (90, 0, 0),
#     ),
# )
## 高脚杯
mesh = scene.add_entity(
    morph = gs.morphs.Mesh(
        file = "meshes/cups/wineglass/Wineglass.obj",
        scale = 0.02,
        pos = (0.66, 0.10, 0.06),
        euler = (90, 0, 0),
    ),
)

########################## 构建 ##########################
scene.build()

# 关节参数
jnt_names = [
    'left_hip_yaw_joint', # 0
    'left_hip_roll_joint', # 1
    'left_hip_pitch_joint', # 2
    'left_knee_joint', # 3
    'left_ankle_joint', # 4
    'right_hip_yaw_joint', # 5
    'right_hip_roll_joint', # 6
    'right_hip_pitch_joint', # 7
    'right_knee_joint', # 8
    'right_ankle_joint', # 9
    'torso_joint', # 10
    'left_shoulder_pitch_joint', # 11
    'left_shoulder_roll_joint', # 12
    'left_shoulder_yaw_joint', # 13
    'left_elbow_joint', # 14
    'left_hand_joint', # 15
    'L_thumb_proximal_yaw_joint', # 16
    'L_thumb_proximal_pitch_joint', # 17
    'L_thumb_intermediate_joint', # 18
    'L_thumb_distal_joint', # 19
    'L_index_proximal_joint', # 20
    'L_index_intermediate_joint', # 21
    'L_middle_proximal_joint', # 22
    'L_middle_intermediate_joint', # 23
    'L_ring_proximal_joint', # 24
    'L_ring_intermediate_joint', # 25
    'L_pinky_proximal_joint', # 26
    'L_pinky_intermediate_joint', # 27
    'right_shoulder_pitch_joint', # 28
    'right_shoulder_roll_joint', # 29
    'right_shoulder_yaw_joint', # 30
    'right_elbow_joint', # 31
    'right_hand_joint', # 32
    'R_thumb_proximal_yaw_joint', # 33
    'R_thumb_proximal_pitch_joint', # 34
    'R_thumb_intermediate_joint', # 35
    'R_thumb_distal_joint', # 36
    'R_index_proximal_joint', # 37
    'R_index_intermediate_joint', # 38
    'R_middle_proximal_joint', # 39
    'R_middle_intermediate_joint', # 40
    'R_ring_proximal_joint', # 41
    'R_ring_intermediate_joint', # 42
    'R_pinky_proximal_joint', # 43
    'R_pinky_intermediate_joint', # 44
]
dofs_idx = [robot.get_joint(name).dof_idx_local for name in jnt_names]

# 设置控制增益
robot.set_dofs_kp(
    kp = np.array([300 for _ in range(45)]),
    dofs_idx_local = dofs_idx,
)
robot.set_dofs_kv(
    kv = np.array([100 for _ in range(45)]),
    dofs_idx_local = dofs_idx,
)
robot.set_dofs_force_range(
    lower = np.array([-30 for _ in range(45)]),
    upper = np.array([ 30 for _ in range(45)]),
    dofs_idx_local = dofs_idx,
)

end_effector = robot.get_link('right_hand_link')

# IK
qpos = robot.inverse_kinematics(
    link = end_effector,
    pos  = np.array([0.55, 0.0, 0.05]),
)
qpos[-12:] = 0

# 规划路径
path = robot.plan_path(
    qpos_goal     = qpos,
    num_waypoints = 120, # 1.2秒时长
    ignore_collision = True,
)
print(path)

def write_dofs(dofs, file):
    for dof in dofs:
        file.write(f'{dof} ')
    file.write('\n')

with open('./anim.txt', 'w') as file:
    for waypoint in path:
        robot.control_dofs_position(waypoint)
        robot.control_dofs_position(np.array([1.08]), np.array([dofs_idx[-12]]))
        write_dofs(robot.get_dofs_position(dofs_idx), file)
        scene.step()
        
    for i in range(100):
        robot.control_dofs_position(np.array([1.08]), np.array([dofs_idx[-12]]))
        write_dofs(robot.get_dofs_position(dofs_idx), file)
        scene.step()

    # for i in range(10000):
    #     if i < 100:
    #         robot.set_dofs_position(waypoint)
    #         robot.set_dofs_position(np.array([0.01 * i]), np.array([dofs_idx[-12]]))
    #         robot.set_dofs_position(np.array([0.004 * i]), np.array([dofs_idx[-11]]))
    #         robot.set_dofs_position(np.array([0.0125 * i for _ in range(8)]), dofs_idx[-8:])
    #     else:
    #         robot.set_dofs_position(waypoint)
    #         robot.set_dofs_position(np.array([1]), np.array([dofs_idx[-12]]))
    #         robot.set_dofs_position(np.array([0.4]), np.array([dofs_idx[-11]]))
    #         robot.set_dofs_position(np.array([1.25 for _ in range(8)]), dofs_idx[-8:])
    #     scene.step()

    for i in range(100):
        robot.control_dofs_position(waypoint)
        robot.control_dofs_position(np.array([1.08]), np.array([dofs_idx[-12]]))
        # robot.control_dofs_position(np.array([0.003 * i]), np.array([dofs_idx[-11]]))
        # robot.control_dofs_position(np.array([0.008 * i for _ in range(8)]), dofs_idx[-8:])
        robot.control_dofs_force(np.array([0.5]), np.array([dofs_idx[-11]]))
        robot.control_dofs_force(np.array([0.5 for _ in range(10)]), dofs_idx[-10:])
        write_dofs(robot.get_dofs_position(dofs_idx), file)
        scene.step()

    qpos = robot.inverse_kinematics(
        link = end_effector,
        pos  = np.array([0.55, -0.2, 0.25]),
    )
    path = robot.plan_path(
        qpos_goal     = qpos,
        num_waypoints = 150,
        ignore_collision = True,
    )
    print(path)
    for waypoint in path:
        robot.control_dofs_position(waypoint)
        robot.control_dofs_force(np.array([0.5]), np.array([dofs_idx[-11]]))
        robot.control_dofs_force(np.array([0.5 for _ in range(10)]), dofs_idx[-10:])
        write_dofs(robot.get_dofs_position(dofs_idx), file)
        scene.step()
    # for i in range(100000):
    #     scene.step()