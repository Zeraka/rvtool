#include <stdio.h>

class X {
    struct Y; // this one is private!
public:
    static void fx();
};

struct X::Y {
    void fy(Y *arg);
};

// this is a member of a private class, thus Y cannot be used in TJP so far
void X::Y::fy(Y *arg) {
} 

// this function may call fy
void X::fx () {
    Y y;
    y.fy (&y);
}

aspect Bug316 {
  advice execution ("% ...::f%(...)") : around () {
    printf ("e1: before %s\n",JoinPoint::signature ());
    tjp->proceed ();
    printf ("e1: after %s\n",JoinPoint::signature ());
  }
  advice execution ("% ...::f%(...)") : around () {
    printf ("e2: before %s\n",JoinPoint::signature ());
    tjp->proceed ();
    printf ("e2: after %s\n",JoinPoint::signature ());
  }
  // this call advice still does not work!
//   advice call ("% ...::f%(...)") : around () {
//     printf ("c1: before %s\n",JoinPoint::signature ());
//     tjp->proceed ();
//     printf ("c1: after %s\n",JoinPoint::signature ());
//   }
//   advice call ("% ...::f%(...)") : around () {
//     printf ("c2: before %s\n",JoinPoint::signature ());
//     tjp->action ().trigger ();
//     printf ("c2: after %s\n",JoinPoint::signature ());
//   }
};

int main () {
    X::fx ();
    return 0;
}

// moegliche Loesung ...

// #include <stdio.h>

// template <int>
// struct TJP {};


// class X {
//     template <int> friend struct TJP;
//     struct Y; // this one is private!
// public:
//     static void fx();
// };

// struct X::Y {
//     void old_fy();
//     void fy();
// };

// template <> struct TJP<1> {
//     typedef X::Y That;
//     typedef X::Y Target;
//     X::Y *_that;
//     X::Y *that () { return _that; }
//     X::Y *target () { return 0; }
//     static const char *signature () { return "X::Y::fy"; }
//     void proceed () { _that->old_fy (); }
// };

// template <typename JP> void adv (JP *tjp) {
//     printf ("before %s\n", JP::signature ());
//     tjp->proceed ();
// }

// // this is a member of a private class, thus Y cannot be used in TJP so far
// void X::Y::fy() {
//     TJP<1> tjp = { this };
//     adv< TJP<1> >(&tjp);
// } 
// // this is a member of a private class, thus Y cannot be used in TJP so far
// void X::Y::old_fy() {
//     printf ("X::Y::old_fy\n");
// } 

// // this function may call fy
// void X::fx () {
//     Y y;
//     y.fy ();
// }

// int main () {
//     X::fx ();
//     return 0;
// }
