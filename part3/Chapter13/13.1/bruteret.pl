#!/usr/bin/perl

for($i = 1; $i < 1500; $i++) {
  print "Attempt $i \n";
  system("./expl_stack1 $i");
}
