package edu.vtc.huffman

import java.io.{BufferedInputStream, FileInputStream}

import scala.annotation.tailrec
import scala.collection.immutable.TreeMap

object Main {

  /**
   * Read the specified file. The file is read in binary mode so any type of file can be read.
   *
   * @param name The name of the file to read.
   * @return A list of byte values between 0 and 255.
   */
  private def readFile(name: String): List[Int] = {
    val reader = new BufferedInputStream(new FileInputStream(name))
    var value = 0
    var result = List[Int]()

    while ({ value = reader.read(); value != -1 }) {
      result = value :: result
    }
    reader.close()
    result.reverse
  }


  /**
   * Return a map that associates a value of zero with keys between 0 and 255 inclusive. The
   * map is not "empty" in the sense that it does have (key, value) pairs. However, it is empty
   * in the sense that each value is zero. This method is used to generate an initial map for
   * byte counting with keys preloaded for all bytes.
   *
   * @return A map with (key, value) pairs for keys from 0 to 255 and associated values of 0.
   */
  private def makeInitialMap: Map[Int, Int] = {

    @tailrec
    def makeInitialFrom(current: Map[Int, Int], n : Int): Map[Int, Int] = {
      n match {
        case 0 => current + (0 -> 0)
        case _ => makeInitialFrom(current + (n -> 0), n - 1)
      }
    }

    makeInitialFrom(TreeMap[Int, Int](), 255)
  }


  /**
   * Finds the CodeTreeNodes with the smallest and second smallest counts in the given set. If
   * the set contains fewer than two items the case object CodeTreeLeaf is used as a placeholder
   * for missing values.
   *
   * @param workingSet The set of nodes to search.
   * @return A pair of code tree nodes with the smallest and second smallest counts.
   */
  private def findTwoSmallest(workingSet: Set[CodeTreeNode]): (CodeTree, CodeTree) = {

    def compareItem(accumulator: (CodeTree, CodeTree), item: CodeTreeNode) = {
      accumulator match {
        case (CodeTreeLeaf, CodeTreeLeaf) => (item, CodeTreeLeaf)

        case (CodeTreeNode(smallest, _, _, _), CodeTreeLeaf) =>
          if (item.count < smallest)
            (item, accumulator._1)
          else
            (accumulator._1, item)

        case (CodeTreeNode(smallest, _, _, _), CodeTreeNode(secondSmallest, _, _, _)) =>
          if (item.count < smallest)
            (item, accumulator._1)
          else if (item.count < secondSmallest)
            (accumulator._1, item)
          else
            accumulator
      }
    }

    workingSet.foldLeft( (CodeTreeLeaf, CodeTreeLeaf): (CodeTree, CodeTree) )(compareItem)
  }


  /**
   * Construct the Huffman code tree from the given initial working set.
   *
   * @param workingSet The set of CodeTreeNodes to use as the base of the tree.
   * @return A CodeTreeNode representing the root of the resulting tree.
   */
  private def buildCodeTree(workingSet: Set[CodeTreeNode]): CodeTreeNode = {

    @tailrec
    def reduceWorkingSet(currentSet: Set[CodeTreeNode]): Set[CodeTreeNode] = {
      if (currentSet.size == 1) currentSet
      else {
        val (smallest: CodeTreeNode, secondSmallest: CodeTreeNode) =
          findTwoSmallest(currentSet)
        val reducedCurrentSet =
          currentSet filterNot { node => (node == smallest) || (node == secondSmallest) }
        val updatedCurrentSet =
          reducedCurrentSet + CodeTreeNode(
            count = smallest.count + secondSmallest.count,
            value = None,
            left  = smallest,
            right = secondSmallest
          )
        reduceWorkingSet(updatedCurrentSet)
      }
    }

    reduceWorkingSet(workingSet).head
  }


  def main(args: Array[String]): Unit = {
    if (args.length != 1) {
      println("usage: java -jar Huffman.jar input_file")
    }
    else {
      val inputData = readFile(args(0))
      val byteCounts = inputData.foldLeft(makeInitialMap)( (accumulatorMap, value) =>
        accumulatorMap + (value -> (accumulatorMap(value) + 1)) )

      // Informational output.
      println("Raw Input = " + inputData)
      println("Byte Counts = " + byteCounts)

      val workingList = for ((key, value) <- byteCounts) yield {
        CodeTreeNode(
          count = value,
          value = Some(key),
          left  = CodeTreeLeaf,
          right = CodeTreeLeaf
        )
      }
      val workingSet = workingList.toSet
      println("WorkingSet = " + workingSet)
      println("Total Counts = " +
        workingSet.foldLeft(0)((accumulator, node) => accumulator + node.count))

      val topLevelNode = buildCodeTree(workingSet)
      println("Top Level Node = " + topLevelNode)

    }
  }

}
