#pragma once

namespace Testing
{
	class TestResult
	{
	public:
		static TestResult Success(std::string testName = __builtin_FUNCTION()) { return { true, "passed", testName }; }
		static TestResult Fail(std::string message = "", std::string testName = __builtin_FUNCTION()) { return { false, message, testName }; }

		bool        success;
		std::string message;
		std::string testName;
	};
}

template <>
struct fmt::formatter<Testing::TestResult>
{
	template <class ParseContext>
	constexpr auto parse(ParseContext& ctx)
	{
		return ctx.begin();
	}
	template <class FormatContext>
	constexpr auto format(const Testing::TestResult& result, FormatContext& a_ctx) const
	{
		if (result.success) {
			return fmt::format_to(a_ctx.out(), "✅ {}", result.testName);
		} else {
			if (result.message.empty()) {
				return fmt::format_to(a_ctx.out(), "❌ {}", result.testName);
			}
			return fmt::format_to(a_ctx.out(), "❌ {}: {}", result.testName, result.message);
		}
	}
};

namespace Testing
{
	class Runner : public ISingleton<Runner>
	{
	private:
		using Code = std::function<void()>;
		using Test = std::function<TestResult()>;
		using TestSuite = std::map<std::string, Test>;
		using TestModule = std::map<std::string, TestSuite>;

		TestModule tests;

		std::map<std::string, Code> beforeAll;
		std::map<std::string, Code> afterAll;

		std::map<std::string, Code> beforeEach;
		std::map<std::string, Code> afterEach;

		static std::pair<int, int> RunModule(std::string moduleName, TestSuite& tests)
		{
			int total = 0;
			int success = 0;

			Code before = [] {};  // no-op
			Code after = [] {};   // no-op

			if (const auto& beforeEach = GetSingleton()->beforeEach.find(moduleName); beforeEach != GetSingleton()->beforeEach.end()) {
				before = beforeEach->second;
			}

			if (const auto& afterEach = GetSingleton()->afterEach.find(moduleName); afterEach != GetSingleton()->afterEach.end()) {
				after = afterEach->second;
			}

			logger::critical("Running {} tests:", moduleName);
			for (auto& test : tests) {
				before();
				auto result = test.second();
				after();
				logger::critical("\t{}", result);
				if (result.success) {
					success++;
				}
				total++;
			}

			logger::critical("Completed {}: {}/{} tests passed", moduleName, success, total);
			return { success, total };
		}

	public:
		static bool RegisterTest(const char* moduleName, const char* testName, Test test)
		{
			auto  runner = GetSingleton();
			auto& module = runner->tests[moduleName];
			return module.try_emplace(testName, test).second;
		}

		static bool RegisterAfterEach(const char* moduleName, Code code)
		{
			return GetSingleton()->afterEach.try_emplace(moduleName, code).second;
		}

		static bool RegisterBeforeEach(const char* moduleName, Code code)
		{
			return GetSingleton()->beforeEach.try_emplace(moduleName, code).second;
		}

		static bool RegisterAfterAll(const char* moduleName, Code code)
		{
			return GetSingleton()->afterAll.try_emplace(moduleName, code).second;
		}

		static bool RegisterBeforeAll(const char* moduleName, Code code)
		{
			return GetSingleton()->beforeAll.try_emplace(moduleName, code).second;
		}

		static void Run()
		{
			LOG_HEADER("SELF TESTING");
			std::pair<int, int> counter = { 0, 0 };
			spdlog::set_level(spdlog::level::critical);  // silence all logging coming from the test.
			for (auto& [moduleName, tests] : GetSingleton()->tests) {
				if (const auto& before = GetSingleton()->beforeAll.find(moduleName); before != GetSingleton()->beforeAll.end()) {
					before->second();
				}
				auto res = RunModule(moduleName, tests);
				if (const auto& after = GetSingleton()->afterAll.find(moduleName); after != GetSingleton()->afterAll.end()) {
					after->second();
				}

				counter.first += res.first;
				counter.second += res.second;
			}
			spdlog::set_level(spdlog::level::info);  // restore logging level
			if (GetSingleton()->tests.size() > 1) {
				logger::info("Completed all tests: {}/{} tests passed", counter.first, counter.second);
			}

			if (counter.first != counter.second) {
				logger::info("No Skyrim for you! Fix Tests first! 😀");
				abort();
			}
		}
	};

	inline void Run() { Runner::Run(); }

	template <typename T>
	inline T* GetForm(RE::FormID a_formID)
	{
		auto form = RE::TESForm::LookupByID<T>(a_formID);
		assert(form);
		return form;
	}
}

#define BEFORE_EACH                                                                                              \
	inline void beforeEachTests();                                                                               \
	static bool beforeEachTests_registered = ::Testing::Runner::RegisterBeforeEach(moduleName, beforeEachTests); \
	inline void beforeEachTests()

#define AFTER_EACH                                                                                            \
	inline void afterEachTests();                                                                             \
	static bool afterEachTests_registered = ::Testing::Runner::RegisterAfterEach(moduleName, afterEachTests); \
	inline void afterEachTests()

#define BEFORE_ALL                                                                                            \
	inline void beforeAllTests();                                                                             \
	static bool beforeAllTests_registered = ::Testing::Runner::RegisterBeforeAll(moduleName, beforeAllTests); \
	inline void beforeAllTests()

#define AFTER_ALL                                                                                          \
	inline void afterAllTests();                                                                           \
	static bool afterAllTests_registered = ::Testing::Runner::RegisterAfterAll(moduleName, afterAllTests); \
	inline void afterAllTests()

#define TEST(name)                                                                                                         \
	inline ::Testing::TestResult test##name();                                                                             \
	static bool                  test##name##_registered = ::Testing::Runner::RegisterTest(moduleName, #name, test##name); \
	inline ::Testing::TestResult test##name##()

#define PASS return ::Testing::TestResult::Success();

#define FAIL(msg) return ::Testing::TestResult::Fail(msg);

#define ASSERT(expr, msg) \
	if (!(expr))          \
		return ::Testing::TestResult::Fail(msg);

#define EXPECT(expr, msg) \
	return (expr) ? ::Testing::TestResult::Success() : ::Testing::TestResult::Fail(msg);

#define WAIT(ms) \
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
