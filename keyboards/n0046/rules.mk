# N0046 编译规则
# 分支一：基础配置
# 分支二：键盘输入
PRODUCT_NAME = n0046
SRC += keyboards/n0046/n0046.c
SRC += keyboards/n0046/keymaps/default.c
INC += keyboards/n0046
INC += keyboards/n0046/keymaps

# 功能开关
BLUETOOTH_ENABLE = yes
EXTRAKEY_ENABLE = yes  # 启用多媒体键支持

