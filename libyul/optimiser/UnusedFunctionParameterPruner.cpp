/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * UnusedFunctionParameterPruner: Optimiser step that removes unused-parameters function.
 */

#include <libyul/optimiser/UnusedFunctionParameterPruner.h>
#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/NameDisplacer.h>
#include <libyul/YulString.h>
#include <libyul/AsmData.h>
#include <libyul/Dialect.h>
#include <libyul/Exceptions.h>

#include <libsolutil/CommonData.h>

#include <algorithm>

using namespace std;
using namespace solidity::util;
using namespace solidity::yul;
using namespace solidity::langutil;

namespace
{

bool anyFalse(vector<bool> const& _mask)
{
	return any_of(_mask.begin(), _mask.end(), [](bool b){ return !b; });
}

template<typename T>
vector<T> applyBooleanMask(vector<T> const& _vec, vector<bool> const& _mask)
{
	yulAssert(_vec.size() == _mask.size(), "");

	vector<T> ret;

	for (size_t i = 0; i < _mask.size(); ++i)
		if (_mask[i])
			ret.push_back(_vec[i]);

	return ret;
}

/**
 * Step 1A of UnusedFunctionParameterPruner: Find functions whose return parameters are not used in
 * the code, i.e.,
 *
 * Look at all VariableDeclaration `let x_1, ..., x_i, ... x_n := f(y_1, ..., y_m)` and check if
 * `x_i` is unused. If all function calls to `f` has its i-th return parameter unused, we will mark
 * its `i`th index so that it can be removed in later steps.
 */
class FindFunctionsWithUnusedReturns: public ASTWalker
{
public:
	explicit FindFunctionsWithUnusedReturns(
		Dialect const& _dialect,
		Block const& _block,
		map<YulString, size_t> const& _references
	) :
		m_dialect(_dialect),
		m_references(_references)

	{
		(*this)(_block);
	}
	using ASTWalker::operator();
	void operator()(VariableDeclaration const& _v) override;

	map<YulString, vector<bool>> m_returns;
 private:
	Dialect const& m_dialect;
	map<YulString, size_t> const& m_references;
};

// Find functions whose return parameters are unused. Assuming that the code is in SSA form and that
// ForLoopConditionIntoBody, ExpressionSplitter were run, all FunctionCalls with at least one return
// value will have the form `let x_1, ..., x_n := f(y_1, ..., y_m)`
void FindFunctionsWithUnusedReturns::operator()(VariableDeclaration const& _v)
{
	if (holds_alternative<FunctionCall>(*_v.value))
	{
		YulString const& name = (get<FunctionCall>(*_v.value)).functionName.name;

		if (m_dialect.builtin(name))
			return;

		if (!m_returns.count(name))
			m_returns[name].resize(_v.variables.size(), false);

		for (size_t i = 0; i < _v.variables.size(); ++i)
			m_returns.at(name)[i] =
				m_returns.at(name)[i] || m_references.count(_v.variables[i].name);
	}
}

/**
 * Step 1B of UnusedFunctionParameterPruner:
 *
 * Find functions whose parameters (both arguments and return-parameters) are not used in its body.
 */
class FindFunctionsWithUnusedParameters
{
public:
	explicit FindFunctionsWithUnusedParameters(
		Dialect const& _dialect,
		Block const& _block,
		map<YulString, size_t> const& _references,
		map<YulString, vector<bool>>& _returns

	) :
		m_returns(_returns),
		m_references(_references),
		m_dialect(_dialect)
	{
		for (auto const& statement: _block.statements)
			if (holds_alternative<FunctionDefinition>(statement))
				(*this)(std::get<FunctionDefinition>(statement));

		for (auto const& [functionName, mask]: m_returns)
			if (anyFalse(mask))
				returnMasks[functionName] = mask;
	}

	void operator()(FunctionDefinition const& _f);

	/// Function name and a boolean mask, where `false` at index `i` indicates that the function
	/// argument at index `i` in `FuntionDefinition::parameters` is unused.
	map<YulString, vector<bool>> argumentMasks;
	/// Function name and a boolean mask, where `false` at index `i` indicates that the
	/// return-parameter at index `i` in `FuntionDefinition::returnVariables` is unused.
	map<YulString, vector<bool>> returnMasks;
	/// A set of functions that could potentially be already pruned by
	/// UnusedFunctionParameterPruner, and therefore should be skipped when applying the
	/// transformation.
	set<YulString> alreadyPruned;

private:
	/// A heuristic to determine if a function was already rewritten by UnusedFunctionParameterPruner
	bool wasPruned(FunctionDefinition const& _f);

	map<YulString, vector<bool>>& m_returns;
	map<YulString, size_t> const& m_references;
	Dialect const& m_dialect;
};

// Find functions whose arguments are not used in its body. Also, find functions whose body
// satisfies a heuristic about pruning.
void FindFunctionsWithUnusedParameters::operator()(FunctionDefinition const& _f)
{
	if (wasPruned(_f))
	{
		alreadyPruned.insert(_f.name);
		return;
	}

	auto contains = [&](auto v) -> bool { return m_references.count(v.name); };

	vector<bool> _argumentMask = applyMap(_f.parameters, contains);
	if (anyFalse(_argumentMask))
		argumentMasks[_f.name] = move(_argumentMask);

	if (!m_returns.count(_f.name))
		m_returns[_f.name].resize(_f.returnVariables.size(), true);

	for (size_t i = 0; i < _f.returnVariables.size(); ++i)
		if (!contains(_f.returnVariables[i]))
			m_returns.at(_f.name)[i] = false;
}


bool FindFunctionsWithUnusedParameters::wasPruned(FunctionDefinition const& _f)
{
	// We skip if the function body if it
	// 1. is empty, or
	// 2. is a single statement that is an assignment statement whose value is a non-builtin
	//    function call, or
	// 3. is a single expression-statement that is a non-builtin function call.
	// The above cases are simple enough so that the inliner alone can remove the parameters.
	if (_f.body.statements.empty())
		return true;
	if (_f.body.statements.size() == 1)
	{
		Statement const& e = _f.body.statements[0];
		if (holds_alternative<Assignment>(e))
		{
			if (holds_alternative<FunctionCall>(*get<Assignment>(e).value))
			{
				FunctionCall c = get<FunctionCall>(*get<Assignment>(e).value);
				if (!m_dialect.builtin(c.functionName.name))
					return true;
			}
		}
		else if (holds_alternative<ExpressionStatement>(e))
			if (holds_alternative<FunctionCall>(get<ExpressionStatement>(e).expression))
			{
				FunctionCall c = get<FunctionCall>(get<ExpressionStatement>(e).expression);
				if (!m_dialect.builtin(c.functionName.name))
					return true;
			}
	}

	return false;
}

/**
 * Step 3 of UnusedFunctionParameterPruner: introduce a new function in the block with body of
 * the old one. Replace the body of the old one with a function call to the new one with reduced
 * parameters.
 *
 * For example: introduce a new function `f` with the same the body as `f_1`, but with reduced
 * parameters, i.e., `function f() -> y { y := 1 }`. Now replace the body of `f_1` with a call to
 * `f`, i.e., `f_1(x) -> y { y := f() }`.
 */
class AddPrunedFunction
{
public:
	explicit AddPrunedFunction(
		map<YulString, vector<bool>> const& _argumentMask,
		map<YulString, vector<bool>> const& _returnMask,
		map<YulString, YulString> const&  _translations,
		map<YulString, size_t> const& _references
	):
		m_references(_references),
		m_argumentMask(_argumentMask),
		m_returnMask(_returnMask),
		m_translations(_translations),
		m_inverseTranslations(invertMap(m_translations))
	{}

	void operator()(Block& _block)
	{
		iterateReplacing(_block.statements, [&](Statement& _s) -> optional<vector<Statement>>
		{
			if (holds_alternative<FunctionDefinition>(_s))
			{
				FunctionDefinition& old = get<FunctionDefinition>(_s);
				if (m_inverseTranslations.count(old.name))
					return addFunction(old);
			}

			return nullopt;
		});
	}

private:
	vector<Statement> addFunction(FunctionDefinition& _old);

	map<YulString, size_t> const& m_references;

	map<YulString, vector<bool>> const& m_argumentMask;
	map<YulString, vector<bool>> const& m_returnMask;

	map<YulString, YulString> const& m_translations;
	map<YulString, YulString> m_inverseTranslations;
};

vector<Statement> AddPrunedFunction::addFunction(FunctionDefinition& _old)
{
	SourceLocation loc = _old.location;
	YulString newName = m_inverseTranslations.at(_old.name);
	TypedNameList functionArguments;
	TypedNameList returnVariables;

	if (m_argumentMask.count(newName))
	{
		vector<bool> const& mask = m_argumentMask.at(newName);
		functionArguments = applyBooleanMask(_old.parameters, mask);
	}
	else
		functionArguments = _old.parameters;

	if (m_returnMask.count(newName))
	{
		vector<bool> const& mask = m_returnMask.at(newName);
		returnVariables = applyBooleanMask(_old.returnVariables, mask);

		// Declare the missing variables on top of the function, if the variable was used, according
		// to `m_references`.
		VariableDeclaration v{loc, {}, nullptr};
		for (size_t i = 0; i < mask.size(); ++i)
			if (!mask[i] && m_references.count(_old.returnVariables[i].name))
				v.variables.emplace_back(_old.returnVariables[i]);

		if (!v.variables.empty())
		{
			Block block{loc, {}};
			block.statements.emplace_back(move(v));
			block.statements += move(_old.body.statements);
			swap(_old.body, block);
		}
	}
	else
		returnVariables = _old.returnVariables;

	FunctionDefinition newFunction{
		loc,
		move(newName),
		move(functionArguments),
		move(returnVariables),
		{loc, {}} // body
	};

	swap(newFunction.body, _old.body);

	FunctionCall call{loc, Identifier{loc, newFunction.name}, {}};
	for (auto const& p: newFunction.parameters)
		call.arguments.emplace_back(Identifier{loc, p.name});

	// Replace the body of `f_1` by an assignment which calls `f`, i.e.,
	// `return_parameters = f(reduced_parameters)`
	if (!newFunction.returnVariables.empty())
	{
		Assignment assignment;
		assignment.location = loc;

		// The LHS of the assignment.
		for (auto const& r: newFunction.returnVariables)
			assignment.variableNames.emplace_back(Identifier{loc, r.name});

		assignment.value = make_unique<Expression>(move(call));

		_old.body.statements.emplace_back(move(assignment));
	}
	else
		_old.body.statements.emplace_back(ExpressionStatement{loc, move(call)});

	return make_vector<Statement>(move(newFunction), move(_old));
}

} // anonymous namespace

void UnusedFunctionParameterPruner::run(OptimiserStepContext& _context, Block& _block)
{
	map<YulString, size_t> references =  ReferencesCounter::countReferences(_block);
	FindFunctionsWithUnusedReturns findReturns{_context.dialect, _block, references};
	FindFunctionsWithUnusedParameters find{_context.dialect, _block, references, findReturns.m_returns};

	auto first = [](auto const& p) { return p.first; };
	set<YulString> functionsWithUnusedArguments
		= applyMap(find.argumentMasks, first, set<YulString>{});
	set<YulString> functionsWithUnusedReturns
		= applyMap(find.returnMasks, first, set<YulString>{});
	set<YulString> namesToFree =
		functionsWithUnusedArguments + functionsWithUnusedReturns - find.alreadyPruned;

	// Step 2 of UnusedFunctionParameterPruner: replace all references to functions with unused
	// parameters with a new name.
	NameDisplacer replace{_context.dispenser, namesToFree};
	replace(_block);

	AddPrunedFunction add{find.argumentMasks, find.returnMasks, replace.translations(), references};
	add(_block);
}
