# Introduction

Dtest is a tool that allows you to write unit tests for distributed
applications and run them without root privileges as a part of the existing
test suite.  Dtest uses Linux network namespaces to trick application processes
into running on the real cluster with each "node" having its own host name and
IP-address, when in fact they run on the single node.  Dtest collects
stdout/stder output from the application processes, prepends host name to each
line and runs tests that searches the lines for the sequence of events. If the
sequence is found, then the test succeeds and Dtest moves to the next one.
After the last test the tool terminates.

Unit tests can be written either in C++ (native) or Python.
The basic idea is to parse lines from stdout/stderr of all the processes that
run on the cluster and find markers that signify specific events. A marker
is any string you like, but it should be unique in order to be useful in tests.
Also, you have to take care to remove these markers from the final version
of the programme via conditional compilation or other means.

# Example

Here is an example of the test that uses Python interface:
```python
import dtest
dtest.cluster(name="x",size=2) # create two-node cluster
dtest.exit_code("all") # check that all processes exit successfully
dtest.add_process([0,1], ["hostname"]) # run hostname command on 0 and 1 nodes
dtest.add_test('hostname 1 is correct', lambda lines: dtest.expect_event_sequence(lines, ['^x1: x1$']))
dtest.add_test('hostname 2 is correct', lambda lines: dtest.expect_event_sequence(lines, ['^x2: x2$']))
dtest.run()
```
Here we create a cluster from two nodes and run `hostname` command on each of them.
Then we check that the output is correct.
The test can be run using the following command.
```bash
dtest-python hostname_test.py
```

# License

Dtest is dual-licensed under GPL3+ and LGPL3+.
