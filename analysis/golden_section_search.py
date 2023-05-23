import typing as ty

GOLDEN_RATIO_INV = (5 ** 0.5 - 1) / 2


def golden_section_search(low: int, high: int, query_func: ty.Callable[[int], float]) -> int:
    if (high - low) == 4:
        hmid = high - 1
        lmid = low + 1
    else:
        interval = int(round((high - low) * GOLDEN_RATIO_INV))
        hmid = low + interval
        lmid = high - interval

    hmid_val = query_func(hmid)
    lmid_val = query_func(lmid)

    while hmid > lmid:
        if hmid_val > lmid_val:
            high = hmid
            hmid = lmid
            lmid = low + int(round((hmid - low) * GOLDEN_RATIO_INV))
            hmid_val = lmid_val
            lmid_val = query_func(lmid)
        else:
            low = lmid
            lmid = hmid
            hmid = high - int(round((high - lmid) * GOLDEN_RATIO_INV))
            lmid_val = hmid_val
            hmid_val = query_func(hmid)

    assert hmid == lmid
    return hmid
