struct Row { bool Temporary{}; };
template <typename T> struct Temporary : T {};
int main() {
  Temporary<Row> row;
  row.Temporary = true;
}
