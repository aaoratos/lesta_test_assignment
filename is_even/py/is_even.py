import sys


def is_even(n: int) -> bool:
    return not bool(n & 1)


def main() -> None:
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <number>")
        sys.exit(1)

    try:
        n = int(sys.argv[1])
    except ValueError:
        print(f"Invalid argument '{sys.argv[1]}', expected a number.")
        sys.exit(1)

    print("Number {} is {}".format(n, "Even" if is_even(n) else "Odd"))


if __name__ == "__main__":
    main()
