class X
{
public:
  static char Y;
  static char GetY();
};

char X::Y = X::GetY();

char X::GetY()
{
  return( 'c' );
}
