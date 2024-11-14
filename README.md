# Lang

A language written in C (currently has no name). This language is interpreted, indentation-based, with static types. Started: 13 Oct 20:50

## Installation

Clone the repo and execute "build.sh"

```bash
git clone https://github.com/agayevhuseyn/lang.git
cd lang
./build.sh
```
For Windows, you should add --windows flag:
```bash
./build.sh --windows
```

## Usage
Here is the standard "hello world" program:
```ada
write("hello, world")
```
Print prime numbers in a certain range:
```ada
function bool is_prime(int n)
	if n == 0 or n == 1
		return false
	for int i = 2; i <= n / 2; i += 1
		if n % i == 0
			return false
	return true

for int i = 0; i < 1000; i += 1
	if is_prime(i)
		write(i)
```
Commenting:
```ada
~ single line comment

~ Comment block should start with double tilde:
~~
and also end with:
~~
```
Skip assignment and assign all variables at one line:
```ada
int a = 10, b, c
int d = c = b = a ~ is equal to d = (c = (b = a))
write(a, b, c, d)
```

## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License
