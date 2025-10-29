#!/bin/bash

# --- Configuration (必须与安装脚本中的一致) ---
SERVICE_NAME="vcan_manager.service"
SCRIPT_NAME="vcan_manager.sh"
INSTALL_SCRIPT_PATH="/usr/local/bin/${SCRIPT_NAME}"
SYSTEMD_SERVICE_PATH="/etc/systemd/system/${SERVICE_NAME}"
LOG_FILE="/var/log/${SCRIPT_NAME%.*}.log" # 例如：/var/log/vcan_manager.log

# --- Functions ---

log_info() {
  echo "[INFO] $(date '+%Y-%m-%d %H:%M:%S') - $1"
}

log_error() {
  echo "[ERROR] $(date '+%Y-%m-%d %H:%M:%S') - $1" >&2
  # 不退出，尝试清理所有可能的组件
}

# 检查当前用户是否为 root
check_root() {
  if [[ $EUID -ne 0 ]]; then
    log_error "This script must be run as root. Please use 'sudo'."
  fi
}

# --- Main Uninstallation Process ---
check_root

log_info "Starting vcan Manager Service uninstallation..."

# 停止并禁用服务
if sudo systemctl is-active --quiet "${SERVICE_NAME}"; then
  log_info "Stopping ${SERVICE_NAME}..."
  sudo systemctl stop "${SERVICE_NAME}" || log_error "Failed to stop ${SERVICE_NAME}."
fi

if sudo systemctl is-enabled --quiet "${SERVICE_NAME}"; then
  log_info "Disabling ${SERVICE_NAME}..."
  sudo systemctl disable "${SERVICE_NAME}" || log_error "Failed to disable ${SERVICE_NAME}."
fi

# 删除 Systemd 服务文件
if [[ -f "${SYSTEMD_SERVICE_PATH}" ]]; then
  log_info "Deleting Systemd service file: ${SYSTEMD_SERVICE_PATH}..."
  sudo rm -f "${SYSTEMD_SERVICE_PATH}" || log_error "Failed to delete ${SYSTEMD_SERVICE_PATH}."
fi

# 重新加载 Systemd 配置
log_info "Reloading Systemd daemon..."
sudo systemctl daemon-reload || log_error "Failed to reload Systemd daemon."

# 删除脚本文件
if [[ -f "${INSTALL_SCRIPT_PATH}" ]]; then
  log_info "Deleting script file: ${INSTALL_SCRIPT_PATH}..."
  sudo rm -f "${INSTALL_SCRIPT_PATH}" || log_error "Failed to delete ${INSTALL_SCRIPT_PATH}."
fi

# 删除日志文件 (可选，如果你想保留日志进行调试，可以注释掉这行)
if [[ -f "$LOG_FILE" ]]; then
  log_info "Deleting log file: ${LOG_FILE}..."
  sudo rm -f "$LOG_FILE" || log_error "Failed to delete ${LOG_FILE}."
fi


log_info "Uninstallation complete! Service ${SERVICE_NAME} and associated files have been removed."
log_info "Note: Virtual CAN interfaces that were created might still be present until reboot or manual removal (e.g., ip link del dev vcan0)."