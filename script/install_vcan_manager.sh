#!/bin/bash

# --- Configuration ---
SERVICE_NAME="vcan_manager.service"
SCRIPT_NAME="vcan_manager.sh"
INSTALL_SCRIPT_PATH="/usr/local/bin/${SCRIPT_NAME}" # 脚本安装到 /usr/local/bin
SYSTEMD_SERVICE_PATH="/etc/systemd/system/${SERVICE_NAME}" # Systemd 服务文件安装到 /etc/systemd/system
LOG_DIR="/var/log" # VCAN Manager 脚本的日志文件目录
LOG_FILE="${LOG_DIR}/${SCRIPT_NAME%.*}.log" # 例如：/var/log/vcan_manager.log

# --- Functions ---

log_info() {
  echo "[INFO] $(date '+%Y-%m-%d %H:%M:%S') - $1"
}

log_error() {
  echo "[ERROR] $(date '+%Y-%m-%d %H:%M:%S') - $1" >&2
  exit 1
}

# 检查当前用户是否为 root
check_root() {
  if [[ $EUID -ne 0 ]]; then
    log_error "This script must be run as root. Please use 'sudo'."
  fi
}

# 验证源文件是否存在
check_source_files() {
  if [[ ! -f "./${SCRIPT_NAME}" ]]; then
    log_error "Error: Source script './${SCRIPT_NAME}' not found in the current directory."
  fi
  if [[ ! -f "./${SERVICE_NAME}" ]]; then
    log_error "Error: Source service file './${SERVICE_NAME}' not found in the current directory."
  fi
  log_info "Source files './${SCRIPT_NAME}' and './${SERVICE_NAME}' found."
}

# --- Main Installation Process ---
check_root
check_source_files

log_info "Starting vcan Manager Service installation..."

# 1. 确保日志目录存在
sudo mkdir -p "${LOG_DIR}" || log_error "Failed to create directory ${LOG_DIR}."

# 2. 复制 vcan_manager.sh 脚本并设置权限
log_info "Copying ${SCRIPT_NAME} to ${INSTALL_SCRIPT_PATH}..."
sudo cp "./${SCRIPT_NAME}" "${INSTALL_SCRIPT_PATH}" || log_error "Failed to copy ${SCRIPT_NAME}."
sudo chmod +x "${INSTALL_SCRIPT_PATH}" || log_error "Failed to set execute permission for ${INSTALL_SCRIPT_PATH}."
log_info "${SCRIPT_NAME} copied and permissions set."

# 3. 复制 vcan_manager.service 文件
log_info "Copying ${SERVICE_NAME} to ${SYSTEMD_SERVICE_PATH}..."
sudo cp "./${SERVICE_NAME}" "${SYSTEMD_SERVICE_PATH}" || log_error "Failed to copy ${SERVICE_NAME}."
log_info "${SERVICE_NAME} copied."

# 4. 重新加载 Systemd 配置
log_info "Reloading Systemd daemon..."
sudo systemctl daemon-reload || log_error "Failed to reload Systemd daemon."

# 5. 启用并启动服务
log_info "Enabling and starting ${SERVICE_NAME}..."
sudo systemctl enable "${SERVICE_NAME}" || log_error "Failed to enable ${SERVICE_NAME}."
sudo systemctl start "${SERVICE_NAME}" || log_error "Failed to start ${SERVICE_NAME}."

log_info "Installation complete! Service ${SERVICE_NAME} is now running and enabled on boot."
log_info "You can check its status with: sudo systemctl status ${SERVICE_NAME}"
log_info "And view its logs with: sudo journalctl -u ${SERVICE_NAME} -f"
log_info "The vcan_manager script will log to: ${LOG_FILE}"