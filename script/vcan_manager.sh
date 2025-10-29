#!/bin/bash

# --- Configuration ---
# 定义要管理的 vcan 接口列表
# 格式: "interface_name:mtu" 或 "interface_name" (使用默认MTU 16)
VCAN_INTERFACES=(
    "can0:16"
    "can1:16"
    "can2"
    "can3"
)

LOG_FILE="/var/log/vcan_manager.log"
CHECK_INTERVAL_SECONDS=1 # 每隔多少秒检查一次 vcan 接口

# --- Logging Function ---
log_message() {
  echo "$(date '+%Y-%m-%d %H:%M:%S') - $1" | tee -a "$LOG_FILE"
}

# --- Main Logic ---

ensure_vcan_module_loaded() {
  log_message "Checking if vcan module is loaded..."
  if ! lsmod | grep -q vcan; then
    log_message "vcan module not loaded. Attempting to load it..."
    if sudo modprobe vcan; then
      log_message "vcan module loaded successfully."
    else
      log_message "ERROR: Failed to load vcan module. Please check permissions or module availability."
      # 尝试继续，因为模块可能被其他方式加载，只是 lsmod 没立即反映
    fi
  else
    log_message "vcan module is already loaded."
  fi
}

get_interface_status() {
  local interface_name="$1"
  local status_output
  status_output=$(ip link show dev "$interface_name" 2>/dev/null)

  if [[ -z "$status_output" ]]; then
    echo "not_exist"
    return
  fi

  if echo "$status_output" | grep -qE 'state UP|state UNKNOWN'; then
    echo "up"
  else
    echo "down"
  fi
}

get_current_mtu() {
  local interface_name="$1"
  local mtu_output
  mtu_output=$(ip link show dev "$interface_name" 2>/dev/null | grep -oP 'mtu \K\d+')
  if [[ -n "$mtu_output" ]]; then
    echo "$mtu_output"
  else
    echo "" # 如果没有找到 MTU 或者接口不存在，返回空
  fi
}


manage_single_vcan_interface() {
  local interface_name="$1"
  local desired_mtu="$2" # 从配置中读取的期望 MTU

  local current_status=$(get_interface_status "$interface_name")
  local current_mtu=$(get_current_mtu "$interface_name")
  local needs_recreation=false # 标志，表示是否需要删除并重新创建

  # 如果 desired_mtu 为空，则表示使用默认 MTU 16
  if [[ -z "$desired_mtu" ]]; then
    desired_mtu=16
  fi

  # 如果当前 MTU 和期望 MTU 不匹配，则标记为需要重新创建
  if [[ -n "$desired_mtu" && "$current_status" == "up" ]]; then
      if [[ "$current_mtu" -ne "$desired_mtu" ]]; then
          log_message "WARN: MTU mismatch for $interface_name. Current: $current_mtu, Desired: $desired_mtu. Will re-create interface."
          needs_recreation=true
      fi
  fi
  # 默认 vcan MTU是16。如果配置没指定，但当前MTU不是16，也重新创建
  if [[ -z "$desired_mtu" && "$current_status" == "up" ]]; then
      if [[ "$current_mtu" -ne 16 ]]; then
          log_message "WARN: Default MTU mismatch for $interface_name. Current: $current_mtu, Desired: 16. Will re-create interface."
          needs_recreation=true
      fi
  fi


  if [[ "$current_status" == "not_exist" || "$needs_recreation" == true ]]; then
    if [[ "$current_status" != "not_exist" ]]; then # 如果接口存在但需要重新创建，先删除
      log_message "Deleting existing $interface_name for re-creation..."
      if ! sudo ip link del dev "$interface_name"; then
        log_message "ERROR: Failed to delete $interface_name. Cannot re-create."
        return 1
      fi
      # 留一点时间给系统清理
      sleep 0.1
    fi

    log_message "Creating $interface_name (type vcan)..."
    if ! sudo ip link add dev "$interface_name" type vcan; then
      log_message "ERROR: Failed to create $interface_name."
      return 1
    fi

    # 如果指定了 MTU，则设置
    if [[ -n "$desired_mtu" ]]; then
      log_message "Setting MTU for $interface_name to $desired_mtu..."
      if ! sudo ip link set "$interface_name" mtu "$desired_mtu"; then
        log_message "WARNING: Failed to set MTU for $interface_name to $desired_mtu. Continuing anyway."
      fi
    fi

    # 启用接口
    log_message "Bringing $interface_name up..."
    if ! sudo ip link set "$interface_name" up; then
      log_message "ERROR: Failed to bring $interface_name up."
      return 1
    fi

    log_message "Successfully created and ensured $interface_name is up with desired configuration."
    return 0
  elif [[ "$current_status" == "down" ]]; then
    log_message "Interface $interface_name exists but is down. Bringing it up..."
    if ! sudo ip link set "$interface_name" up; then
      log_message "ERROR: Failed to bring $interface_name up."
      return 1
    fi
    log_message "Successfully brought $interface_name up."
  else # 接口已经存在且为 UP，且 MTU 匹配
    log_message "Interface $interface_name is already up and configured correctly (MTU: $current_mtu)."
  fi

  return 0
}

# --- Main Loop ---
log_message "vcan Manager Service started (Shell Script)."
ensure_vcan_module_loaded

while true; do
  for vcan_cfg in "${VCAN_INTERFACES[@]}"; do
    interface_name=$(echo "$vcan_cfg" | cut -d':' -f1)
    # 如果没有指定MTU，则使用默认的16，或者保持空让脚本在处理时判断
    desired_mtu=$(echo "$vcan_cfg" | cut -s -d':' -f2) # -s 参数在没有分隔符时不输出错误

    # 调用新的管理函数
    manage_single_vcan_interface "$interface_name" "$desired_mtu"
  done
  sleep "$CHECK_INTERVAL_SECONDS"
done