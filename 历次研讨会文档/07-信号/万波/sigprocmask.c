int sigprocmask(int how, sigset_t *set, sigset_t *oldset)
{
	int error;

	spin_lock_irq(&current->sighand->siglock);
	if (oldset)
		*oldset = current->blocked;

	error = 0;
	switch (how) {
	case SIG_BLOCK:
		sigorsets(&current->blocked, &current->blocked, set);
		break;
	case SIG_UNBLOCK:
		signandsets(&current->blocked, &current->blocked, set);
		break;
	case SIG_SETMASK:
		current->blocked = *set;
		break;
	default:
		error = -EINVAL;
	}
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);

	return error;
}