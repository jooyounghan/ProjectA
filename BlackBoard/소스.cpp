#include <memory>

class Test
{
	std::unique_ptr<int> test;
public:
	void SetTest(std::unique_ptr<int> t);
};

void Test::SetTest(std::unique_ptr<int> t)
{
	test = std::move(t);
}

int main()
{
	Test test;
	std::unique_ptr<int> test1 = std::make_unique<int>(1);
	test.SetTest(std::move(test1));
	bool t = true;

}