#pragma once
#include "Tree.h"
#include "Conditional.h"
#include "Predicate.h"

namespace BehaviorTree
{
	class Tree;

	class TreeBuilder
	{
		template <class ParentType, class CompositeType>
		class CompositeBuilder;

	public:
		template <class ParentType, class BehaviorType>
		class LeafBuilder
		{
			friend class TreeBuilder;

		public:
			LeafBuilder(Tree* tree, ParentType* parent, UpdateCallback update, std::string name)
				: mTree(tree)
				, mParent(parent)
			{
				mBehavior = new BehaviorType(*mTree, name);

				if (update)
				{
					mBehavior->SetUpdateCallback(update);
				}
			}

			template <typename ReturnType, typename ObjectType, typename... ParamTypes>
			LeafBuilder& Invoke(ReturnType(ObjectType::*method)(ParamTypes...), ParamTypes... params)
			{
				(mBehavior->*method)(params...);
				return (*this);
			}

			ParentType& End()
			{
				return *mParent;
			}

		private:
			Tree* mTree;
			ParentType* mParent;
			BehaviorType* mBehavior;
		};

		template <class ParentType>
		class ConditionalBuilder
		{

		public:
			ConditionalBuilder(Tree* tree, ParentType* parent, std::string name)
				: mTree(tree)
				, mParent(parent)
			{
				mConditional = new Conditional(*mTree, nullptr, nullptr, name);
			}

			ConditionalBuilder<ParentType> Action(UpdateCallback update, std::string name = "Action")
			{
				auto builder = LeafBuilder<ConditionalBuilder, BehaviorTree::Behavior>(mTree, this, update, name);
				mConditional->SetChild(builder.mBehavior);
				return *this;
			}

			template <class ChildCompositeType>
			CompositeBuilder<ConditionalBuilder<ParentType>, ChildCompositeType> Composite(std::string name = "Composite")
			{
				auto builder = CompositeBuilder<ConditionalBuilder<ParentType>, ChildCompositeType>(mTree, this, name);
				mConditional->SetChild(builder.mComposite);
				return builder;
			}

			ConditionalBuilder<ParentType> Predicate(PredicateCallback predicate, std::string name = "Predicate")
			{
				auto builder = LeafBuilder<ConditionalBuilder, BehaviorTree::Predicate>(mTree, this, nullptr, name);
				builder.Invoke(&Predicate::SetPredicateCallback, predicate);
				mConditional->SetPredicate(builder.mBehavior);
				return *this;
			}

			ParentType& End()
			{
				return *mParent;
			}

			Tree* mTree;
			ParentType* mParent;
			Conditional* mConditional;
			std::string mName;
		};


		template <class ParentType, class DecoratorType>
		class DecoratorBuilder
		{
			friend class TreeBuilder;

		public:
			DecoratorBuilder(Tree* tree, ParentType* parent, std::string name)
				: mTree(tree)
				, mParent(parent)
			{
				mDecorator = new DecoratorType(*mTree, nullptr, name);
			}

			template <class ChildCompositeType>
			CompositeBuilder<DecoratorBuilder<ParentType, DecoratorType>, ChildCompositeType> Composite(std::string name = "Composite")
			{
				auto builder = CompositeBuilder<DecoratorBuilder<ParentType, DecoratorType>, ChildCompositeType>(mTree, this, name);
				mDecorator->SetChild(builder.mComposite);
				return builder;
			}

			DecoratorBuilder<ParentType, DecoratorType> Action(UpdateCallback update, std::string name = "Action")
			{
				auto builder = LeafBuilder<DecoratorBuilder, BehaviorTree::Behavior>(mTree, this, update, name);
				mDecorator->SetChild(builder.mBehavior);
				return *this;
			}

			DecoratorBuilder<ParentType, DecoratorType> Predicate(PredicateCallback predicate, std::string name = "Predicate")
			{
				auto builder = LeafBuilder<DecoratorBuilder, BehaviorTree::Predicate>(mTree, this, nullptr, name);
				builder.Invoke(&Predicate::SetPredicateCallback, predicate);
				mDecorator->SetChild(builder.mBehavior);
				return *this;
			}

			ConditionalBuilder<DecoratorBuilder<ParentType, DecoratorType>> Conditional(std::string name = "Conditional")
			{
				auto builder = ConditionalBuilder<DecoratorBuilder>(mTree, this, name);
				mDecorator->SetChild(builder.mConditional);
				return builder;
			}

			DecoratorBuilder<ParentType, DecoratorType> Subtree(Tree& tree, std::string name = "Subtree")
			{
				mDecorator->SetChild(tree);
				return *this;
			}

			ParentType& End()
			{
				return *mParent;
			}

		private:
			Tree* mTree;
			ParentType* mParent;
			DecoratorType* mDecorator;
		};

		template <class ParentType, class CompositeType>
		class CompositeBuilder
		{
			friend class TreeBuilder;

		public:
			CompositeBuilder(Tree* tree, ParentType* parent, std::string name)
				: mTree(tree)
				, mParent(parent)
			{
				mComposite = new CompositeType(*mTree, name);
			}

			template <class NodeType, typename... ParamTypes>
			LeafBuilder<CompositeBuilder, NodeType> Node(ParamTypes... params);

			//template <typename... ParamTypes>
			//CompositeBuilder<CompositeBuilder<ParentType>> Composite(ParamTypes... params);

			template <class ChildCompositeType>
			CompositeBuilder<CompositeBuilder<ParentType, CompositeType>, ChildCompositeType> Composite(std::string name = "Composite")
			{
				auto builder = CompositeBuilder<CompositeBuilder<ParentType, CompositeType>, ChildCompositeType>(mTree, this, name);
				mComposite->Add(*builder.mComposite);
				return builder;
			}

			CompositeBuilder<ParentType, CompositeType> Action(UpdateCallback update, std::string name = "Action")
			{
				auto builder = LeafBuilder<CompositeBuilder, BehaviorTree::Behavior>(mTree, this, update, name);
				mComposite->Add(*builder.mBehavior);
				return *this;
			}

			CompositeBuilder<ParentType, CompositeType> Predicate(PredicateCallback predicate, std::string name = "Predicate")
			{
				auto builder = LeafBuilder<CompositeBuilder, BehaviorTree::Predicate>(mTree, this, nullptr, name);
				builder.Invoke(&Predicate::SetPredicateCallback, predicate);
				mComposite->Add(*builder.mBehavior);
				return *this;
			}

			ConditionalBuilder<CompositeBuilder<ParentType, CompositeType>> Conditional(std::string name = "Conditional")
			{
				auto builder = ConditionalBuilder<CompositeBuilder>(mTree, this, name);
				mComposite->Add(*builder.mConditional);
				return builder;
			}

			template <class DecoratorType>
			DecoratorBuilder<CompositeBuilder<ParentType, CompositeType>, DecoratorType> Decorator(std::string name = "Decorator")
			{
				auto builder = DecoratorBuilder<CompositeBuilder, DecoratorType>(mTree, this, name);
				mComposite->Add(*builder.mDecorator);
				return builder;
			}

			CompositeBuilder<ParentType, CompositeType> Subtree(Tree& tree, std::string name = "Subtree")
			{
				mComposite->Add(tree);
				return *this;
			}

			ParentType& End()
			{
				return *mParent;
			}

		private:
			Tree* mTree;
			ParentType* mParent;
			CompositeType* mComposite;
		};


		TreeBuilder(std::string name = "Tree")
		{
			mTree = new Tree(name);
		}

		template <class NodeType>
		LeafBuilder<TreeBuilder, NodeType> Node(std::string name = "Node")
		{
			auto builder = LeafBuilder<TreeBuilder, NodeType>(mTree, this, nullptr, name);
			mTree->Start(*builder.mBehavior);
			return builder;
		}

		LeafBuilder<TreeBuilder, Behavior> Action(UpdateCallback update, std::string name = "Action")
		{
			auto builder = LeafBuilder<TreeBuilder, BehaviorTree::Behavior>(mTree, this, update, name);
			mTree->Start(*builder.mBehavior);
			return builder;
		}


		template <class CompositeType>
		CompositeBuilder<TreeBuilder, CompositeType> Composite(std::string name = "Composite")
		{
			auto builder = CompositeBuilder<TreeBuilder, CompositeType>(mTree, this, name);
			mTree->Start(*builder.mComposite);
			return builder;
		}

		//template <class DecoratorType, typename... ParamTypes>
		//DecoratorBuilder<TreeBuilder> Decorator(ParamTypes... params);


		Tree& End() const
		{
			return *mTree;
		}

	private:
		Tree* mTree;
	};
}
