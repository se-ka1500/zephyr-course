/*
 * Ring Buffer Module - Homework Test Skeleton
 *
 * test_fresh_state is provided as a worked example. Fill in the remaining
 * 7 ZTEST bodies according to TEST_SPEC.md. Stubs call ztest_test_skip()
 * so the binary builds and runs cleanly before each test is implemented.
 *
 * Run:
 *   west twister -T tests/ring_buf -p native_sim
 */

#include <zephyr/ztest.h>
#include <errno.h>

#include "ring_buf.h"

/*
 * Shared before hook: every suite reinitialises the ring buffer with a
 * capacity of 4 so tests start from a clean, known state. Capacity 4 is
 * enough to exercise FIFO order (push 1, 2, 3) and overflow (full at 4).
 */
static void before(void *f)
{
	ARG_UNUSED(f);
	rb_init(4);
}

/*
 * ============================================================================
 * Test Suite: ring_buf_init
 *
 * Initial state and re-initialization behaviour.
 * ============================================================================
 */
ZTEST_SUITE(ring_buf_init, NULL, NULL, before, NULL, NULL);

/* PROVIDED — study this test before writing the rest. */
ZTEST(ring_buf_init, test_fresh_state)
{
	zassert_true(rb_is_empty(), "Fresh buffer must be empty");
	zassert_equal(rb_count(), 0, "Fresh buffer count must be 0");
}

ZTEST(ring_buf_init, test_reinit_clears_state)
{
	int ret;

	ret = rb_push(99);
	zassert_equal(ret, 0, "push should succeed");
	zassert_equal(rb_count(), 1, "count should be 1 after push");

	ret = rb_init(4);
	zassert_equal(ret, 0, "re-init should succeed");
	zassert_true(rb_is_empty(), "buffer must be empty after re-init");
	zassert_equal(rb_count(), 0, "count must be 0 after re-init");
}

/*
 * ============================================================================
 * Test Suite: ring_buf_push_pop
 *
 * Single push/pop round-trip, FIFO order, full error path.
 * ============================================================================
 */
ZTEST_SUITE(ring_buf_push_pop, NULL, NULL, before, NULL, NULL);

ZTEST(ring_buf_push_pop, test_single_push_pop)
{
	int v;
	int ret;

	ret = rb_push(42);
	zassert_equal(ret, 0, "push should succeed");

	ret = rb_pop(&v);
	zassert_equal(ret, 0, "pop should succeed");
	zassert_equal(v, 42, "popped value should be 42");
	zassert_true(rb_is_empty(), "buffer should be empty after pop");
}

ZTEST(ring_buf_push_pop, test_fifo_order)
{
	int v;
	int ret;

	ret = rb_push(1);
	zassert_equal(ret, 0, "push(1) should succeed");
	ret = rb_push(2);
	zassert_equal(ret, 0, "push(2) should succeed");
	ret = rb_push(3);
	zassert_equal(ret, 0, "push(3) should succeed");

	ret = rb_pop(&v);
	zassert_equal(ret, 0, "first pop should succeed");
	zassert_equal(v, 1, "first popped value should be 1");

	ret = rb_pop(&v);
	zassert_equal(ret, 0, "second pop should succeed");
	zassert_equal(v, 2, "second popped value should be 2");

	ret = rb_pop(&v);
	zassert_equal(ret, 0, "third pop should succeed");
	zassert_equal(v, 3, "third popped value should be 3");

	zassert_true(rb_is_empty(), "buffer should be empty after all pops");
}

ZTEST(ring_buf_push_pop, test_push_full_returns_enospc)
{
	int ret;

	ret = rb_push(10);
	zassert_equal(ret, 0, "push(10) should succeed");
	ret = rb_push(20);
	zassert_equal(ret, 0, "push(20) should succeed");
	ret = rb_push(30);
	zassert_equal(ret, 0, "push(30) should succeed");
	ret = rb_push(40);
	zassert_equal(ret, 0, "push(40) should succeed");

	zassert_true(rb_is_full(), "buffer should be full at capacity");

	ret = rb_push(50);
	zassert_equal(ret, -ENOSPC, "push on full buffer should return -ENOSPC");
}

/*
 * ============================================================================
 * Test Suite: ring_buf_boundaries
 *
 * Peek semantics and NULL-pointer boundary conditions.
 * ============================================================================
 */
ZTEST_SUITE(ring_buf_boundaries, NULL, NULL, before, NULL, NULL);

ZTEST(ring_buf_boundaries, test_peek_does_not_consume)
{
	int v;
	int ret;

	ret = rb_push(7);
	zassert_equal(ret, 0, "push should succeed");

	ret = rb_peek(&v);
	zassert_equal(ret, 0, "first peek should succeed");
	zassert_equal(v, 7, "first peek should return 7");
	zassert_equal(rb_count(), 1, "count should still be 1 after first peek");

	ret = rb_peek(&v);
	zassert_equal(ret, 0, "second peek should succeed");
	zassert_equal(v, 7, "second peek should still return 7");
	zassert_equal(rb_count(), 1, "count should still be 1 after second peek");
}

ZTEST(ring_buf_boundaries, test_pop_null_returns_einval)
{
	int ret;

	ret = rb_pop(NULL);
	zassert_equal(ret, -EINVAL, "rb_pop(NULL) should return -EINVAL");
}

ZTEST(ring_buf_boundaries, test_is_full_after_fill)
{
	int ret;

	ret = rb_push(1);
	zassert_equal(ret, 0, "push(1) should succeed");
	ret = rb_push(2);
	zassert_equal(ret, 0, "push(2) should succeed");
	ret = rb_push(3);
	zassert_equal(ret, 0, "push(3) should succeed");
	ret = rb_push(4);
	zassert_equal(ret, 0, "push(4) should succeed");

	zassert_true(rb_is_full(), "buffer should be full after 4 pushes");
	zassert_equal(rb_count(), 4, "count should be 4 after filling buffer");
}
