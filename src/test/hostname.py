import dtest
dtest.cluster(name="x",size=2) # create two-node cluster
dtest.exit_code("all") # check that all processes exit successfully
dtest.add_process([0,1], ["hostname"]) # run hostname command on 0 and 1 nodes
dtest.add_test('hostname 1 is correct', lambda lines: dtest.expect_event_sequence(lines, ['^x1: x1$']))
dtest.add_test('hostname 2 is correct', lambda lines: dtest.expect_event_sequence(lines, ['^x2: x2$']))
dtest.run()

