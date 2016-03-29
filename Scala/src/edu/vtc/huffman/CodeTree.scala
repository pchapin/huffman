package edu.vtc.huffman

sealed abstract class CodeTree

case object CodeTreeLeaf extends CodeTree

case class CodeTreeNode(
  count: Int,
  value: Option[Int],
  left : CodeTree,
  right: CodeTree) extends CodeTree
