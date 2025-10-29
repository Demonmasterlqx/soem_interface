用于监控和保证vcan口是通的

# 使用方式

首先使用 `sudo chmod +x *.sh` 设定权限

可以在 `vcan_manager.sh` 中修改希望管理的can口

```bash
VCAN_INTERFACES=(
    "can0:16"
    "can1:16"
    "can2"
)
```

## 安装

`./install_vcan_manager.sh`

## 卸载

`./uninstall_vcan_manager.sh`