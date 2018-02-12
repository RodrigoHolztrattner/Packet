# Peon, a work stealing queue system.

Originally created to help me with the processing division of the game engines that I work on, it has become an almost essential module in all my projects.

Basically this system works by breaking the code into smaller pieces that can be processed in parallel to increase performance without generating too many dependencies internally. All features are (partially) described in the Peon.h file.
