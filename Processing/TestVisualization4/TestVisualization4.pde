// The Nature of Code
// Daniel Shiffman
// http://natureofcode.com

// Recursive Tree

// Renders a simple tree-like structure via recursion
// Branching angle calculated as a function of horizontal mouse position

float theta;

Table table;

int currentRow;

final int N_ACTIONS = 9;
final int N_BINS = 5;

void setup() {
  size(700, 700);
  smooth();
  
  table = loadTable("report_test1_zerospeed.csv", "csv");
//  table = loadTable("report_test2_reddot.csv", "csv");
  currentRow = 0;
}

void draw() {
  background(255);
  stroke(0);
  
  State state = new State(table.getRow(currentRow));
  state.draw(width/2, height/2, width/6);
}

void keyPressed() {
  if (key == ' ') {
    currentRow = (currentRow+1) % table.getRowCount();
  }
  else if (key == 'r') {
    currentRow = 0;
  }
}
