#\!/bin/bash
passed=0
failed=0
for i in {1..9}; do
    rm -f *.dat
    ./code < /workspace/data/004/1075/$i.in > test_output.txt 2>&1
    if diff -q /workspace/data/004/1075/$i.out test_output.txt > /dev/null 2>&1; then
        echo "Test $i: PASS"
        ((passed++))
    else
        echo "Test $i: FAIL"
        ((failed++))
    fi
done

# Test 10 is the setup test
rm -f *.dat
./code < /workspace/data/004/1075/10.in > /dev/null 2>&1

# Tests 100+ depend on test 10
for i in {100..109}; do
    ./code < /workspace/data/004/1075/$i.in > test_output.txt 2>&1
    if diff -q /workspace/data/004/1075/$i.out test_output.txt > /dev/null 2>&1; then
        echo "Test $i: PASS"
        ((passed++))
    else
        echo "Test $i: FAIL"
        ((failed++))
    fi
done

echo "Passed: $passed, Failed: $failed"
