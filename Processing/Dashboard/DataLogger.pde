class DataLogger {
  Table table;
  boolean isRecording;
  
  DataLogger() {
    clear();
    isRecording = false;
  }
  
  void setRecording(boolean rec) {
    isRecording = rec;
  }
  
  boolean isRecording() { return isRecording; }
  
  Table getTable() { return table; }
  
  int count() {
    return (table == null ? 0 : table.getRowCount());
  }
  
  void clear() {
    table = new Table();
    table.addColumn("time");
    table.addColumn("yaw");
    table.addColumn("pitch");
    table.addColumn("roll");
    //table.addColumn("yaw_rate");
    //table.addColumn("pitch_rate");
    //table.addColumn("roll_rate");
    table.addColumn("motor_speed");
    table.addColumn("tilt_position");
    table.addColumn("motor_ticks");
    table.addColumn("tilt_ticks");
  }
  
  void recordState() {
    if (isRecording()) {
      TableRow row = table.addRow();
      row.setLong("time", millis());
      row.setFloat("yaw", robot.getYaw());
      row.setFloat("pitch", robot.getPitch());
      row.setFloat("roll", robot.getRoll());
      row.setInt("motor_speed", robot.getRollerMotorSpeed());
      row.setInt("tilt_position", robot.getTilterMotorPosition());
      row.setInt("motor_ticks", robot.getRollerMotorTicks());
      row.setInt("tilt_ticks", robot.getTilterMotorTicks());
    }
  }
  
  void save(String fileName) {
    saveTable(table, fileName);
  }
}