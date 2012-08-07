/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#ifndef XMLREADERSAR_H
#define XMLREADERSAR_H

#include <iterator>
#include <stdio.h>

#include "Filename.h"
#include "LocationType.h"
#include "MessageLogMgr.h"
#include "MessageLog.h"
#include "XercesIncludes.h"
#include "xmlbase.h"

class Font;

/** @file xmlreader.h
 *  @brief XML utilities and functionality for reading and parsing
 */

/**
 * This class reads and parses XML.
 * @ingroup app_xml
 *
 * @par requirements
 * Apache Xerces-C++ version 3.1.1
 */
class XmlReaderSAR : public XmlBase
{
public:
   /**
    * Create an %XmlReader.
    *
    * @param pLog
    *        Optional MessageLog to be passed to XmlBase.
    *
    * @param bValidate
    *        Should the %XmlReader perform validation? It is strongly
    *        suggested that you always perform validation except when
    *        this is not possible. (such as during development, before
    *        XSD entries are made)
    */
   XmlReaderSAR(MessageLog* pLog = NULL, bool bValidate = true);

   /**
    * Destroy and cleanup the %XmlReader object.
    */
   ~XmlReaderSAR();

   /**
    * Parse a file.
    *
    * @param pFn
    *        The file to parse, as a Filename. Should not be \c NULL.
    *
    * @param endTag
    *        If not empty, the parse will halt when this end tag is reached.
    *
    * @return The root element. Returns \c NULL for failure.
    */
   XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* parse(const Filename* pFn, std::string endTag = "");

   /**
    * Parse a file.
    *
    * @param fn
    *        The file to parse, as a string.
    *
    * @param endTag
    *        If not empty, the parse will halt when this end tag is reached.
    *
    * @return The root element. Returns \c NULL for failure.
    */
   XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* parse(std::string fn, std::string endTag = "");

   /**
    * Parse a string.  This method should not be used if validation has been
    * enabled for this XmlReader.
    *
    * @param str
    *        The string to parse.
    *
    * @return The root element. Returns \c NULL for failure.
    */
   XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* parseString(const std::string& str);

   /**
    * Perform an XPath query on the parsed document.
    *
    *
    * @param expression
    *        A valid XPath 2.0 expression.
    *
    * @param type
    *        The type of result requested. Allowed values: DOMXPathResult::FIRST_RESULT_TYPE,
    *        DOMXPathResult::ITERATOR_RESULT_TYPE, or DOMXPathResult::SNAPSHOT_RESULT_TYPE.
    *
    * @param reuse
    *        If this is true, the DOMXPathResult will be reused for each query. If this
    *        is false, a new result will be generated. If true, the XmlReader retains
    *        ownership of the result. If false, the caller takes ownership of the result.
    *
    * @return The result of the XPath query. \c NULL if the query failed or no document is loaded.
    */
   XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult* query(const std::string& expression,
      XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::ResultType type, bool reuse = true);

   /**
    * Perform an XPath query on the parsed document using a default namespace.
    *
    *
    * @param expression
    *        A valid XPath 2.0 expression.
	*
	* @param namespace
    *        The default xmlns namespace string.
    *
    * @param type
    *        The type of result requested. Allowed values: DOMXPathResult::FIRST_RESULT_TYPE,
    *        DOMXPathResult::ITERATOR_RESULT_TYPE, or DOMXPathResult::SNAPSHOT_RESULT_TYPE.
    *
    * @param reuse
    *        If this is true, the DOMXPathResult will be reused for each query. If this
    *        is false, a new result will be generated. If true, the XmlReader retains
    *        ownership of the result. If false, the caller takes ownership of the result.
    *
    * @return The result of the XPath query. \c NULL if the query failed or no document is loaded.
    */
   XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult* queryNamespace(const std::string& expression,const std::string DefaultNamespace,
      XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::ResultType type, bool reuse = true);

public: // struct and static utility functions
   /**
    * Convert a string to another type.
    *
    * This templated class converts a string
    * to a template specified type using
    * a std::stringstream.
    *
    * @see std::stringstream
    */
   template<typename T>
   class StringStreamAssigner
   {
   public:
      /**
       * Convert a string to another type.
       *
       * @param pVal
       *        The string representation of a \b T
       *
       * @return The converted \b T. If \e arg does not
       *         contain a valid string representation
       *         of a \b T, the return value is the default
       *         value of \b T.
       */
      T operator()(const char* pVal)
      {
         std::stringstream s(pVal);
         T tmp;
         s >> tmp;
         return tmp;
      }
   };

   /**
    * Convert a const char * to another type.
    *
    * This templated class converts a const char *
    * to a template specified type by first converting
    * it to a std::string. \b T must have a std::string
    * conversion operator or a constructor which takes
    * a std::string.
    */
   template<typename T>
   class StringAssigner
   {
   public:
      /**
       * Convert a const char * to another type.
       *
       * @param pVal
       *        The const char * representation of a \b T
       *
       * @return The converted \b T.
       */
      T operator()(const char* pVal)
      {
         return std::string(pVal);
      }
   };

   /**
    * Convert a const char * to another type.
    *
    * This templated class converts a const char *
    * to a template specified type.
    * \b T must have a constructor which takes a const char *.
    */
   template<typename T>
   class ParseStringAssigner
   {
   public:
      /**
       * Convert a const char * to another type.
       *
       * @param pVal
       *        The const char * representation of a \b T
       *
       * @return The converted \b T. The caller takes ownership.
       */
      T* operator()(const char* pVal)
      {
         return new T(pVal);
      }
   };

   /**
    * Convert a Unicode string to a vector of objects.
    *
    * The Unicode string is tokenized using XML list tokenization
    * and passed into a vector. The individual values are converted
    * from the tokenized strings to \b T using the functor \b B.
    *
    * Example:
    *  @code
    *    vector<int> *pIntVec(NULL);
    *    pIntVec = reinterpret_cast<vector<int>* >(StrToVector<int, StringStreamAssigner<int> >(X("1 2 3 4")));
    *  @endcode
    *
    * @param pStr
    *        The Unicode string to parse.
    *
    * @return A pointer to a \b vector<T> which has
    *         been created with \e new. The caller
    *         takes ownership of this vector.
    */
   template<typename T, class B>
   static void* StrToVector(const XMLCh* pStr)
   {
      std::vector<T>* v(new std::vector<T>);
      XERCES_CPP_NAMESPACE_QUALIFIER XMLStringTokenizer t(pStr);
      B bb;
      while (t.hasMoreTokens())
      {
         v->push_back(bb(A(t.nextToken())));
      }

      return reinterpret_cast<void*>(v);
   }

   /**
    * Convert a Unicode string to a vector of objects.
    *
    * The Unicode string is tokenized using XML list tokenization
    * and passed into a vector. The individual values are converted
    * from the tokenized strings to \b T using the functor \b B.
    *
    * Example:
    *  @code
    *    vector<int> intVec;
    *    StrToVector<int,StringStreamAssigner<int> >(intVec, X("1 2 3 4")));
    *  @endcode
    *
    * @param vec
    *        The vector to place the values in.
    *
    * @param pStr
    *        The Unicode string to parse.
    */
   template<typename T, class B>
   static void StrToVector(typename std::vector<T>& vec, const XMLCh* pStr)
   {
      XERCES_CPP_NAMESPACE_QUALIFIER XMLStringTokenizer t(pStr);
      B bb;
      while (t.hasMoreTokens())
      {
         vec.push_back(bb(A(t.nextToken())));
      }
   }

   /**
    * Convert a Unicode string to a LocationType.
    *
    * The Unicode string is tokenized using XML list tokenization.
    *
    * @param pStr
    *        The Unicode string to parse.
    *
    * @param loc
    *        Output argument. The LocationType in which to store the result.
    */
   static bool StrToLocation(const XMLCh* pStr, LocationType& loc)
   {
      XERCES_CPP_NAMESPACE_QUALIFIER XMLStringTokenizer t(pStr);
      if (t.hasMoreTokens())
      {
         loc.mX = atof(A(t.nextToken()));
         if (t.hasMoreTokens())
         {
            loc.mY = atof(A(t.nextToken()));
            return true;
         }
      }

      return false;
   }

   /**
    * Convert a Unicode string to a quad coordinate.
    *
    * The Unicode string is tokenized using XML list tokenization.
    * The coordinate is represented by the four arguments \e a, \e b, \e c, and \e d.
    * If fewer than four tokens are available, the remaining coordinate values
    * are set to 0.0. This means, it is possible to parse 1, 2, 3 or 4, coordinate
    * lists with the function.
    *
    * Example:
    *  @code
    *     // Parse a quad coord
    *     {
    *       double a,b,c,d;
    *       StrToQuadCoord(X("1.2 3.4 5.6 7.8"), a, b, c, d);
    *     }
    *
    *     // Now parse a tri coord
    *     {
    *       double r, g, b, dummy;
    *       StrToQuadCoord(X("12.4 44.0 16.85"), r, g, b, dummy);
    *     }
    *
    *     // Finally, an example of parsing a double coord
    *     {
    *       double x, y, dummy;
    *       StrToQuadCoord(X("42.43 15.0"), x, y, dummy, dummy);
    *     }
    *
    *  @endcode
    *
    * @param pStr
    *        The Unicode string to parse.
    *
    * @param a
    *        Output argument. The first part of the quad-coord.
    * @param b
    *        Output argument. The second part of the quad-coord.
    * @param c
    *        Output argument. The third part of the quad-coord.
    * @param d
    *        Output argument. The fourth part of the quad-coord.
    */
   static void StrToQuadCoord(const XMLCh* pStr, double& a, double& b, double& c, double& d)
   {
      a = 0.0;
      b = 0.0;
      c = 0.0;
      d = 0.0;

      XERCES_CPP_NAMESPACE_QUALIFIER XMLStringTokenizer t(pStr);
      if (t.hasMoreTokens())
      {
         a = atof(A(t.nextToken()));
      }

      if (t.hasMoreTokens())
      {
         b = atof(A(t.nextToken()));
      }

      if (t.hasMoreTokens())
      {
         c = atof(A(t.nextToken()));
      }

      if (t.hasMoreTokens())
      {
         d = atof(A(t.nextToken()));
      }
   }

   /**
    * An exception class representing an error or warning while reading and parsing.
    */
   class XmlReaderException : public XmlException
   {
   public:
      /**
       * Create a new %XmlReaderException.
       *
       * @param msg
       *        The message associated with this exception.
       *
       * @param binary
       *        The file (or string) being parsed is likely a binary file.
       *        It is easy for the parser to determine when the parse string
       *        is likely a binary (or non-XML) file. Since the application supports
       *        loading of some older binary files, this is useful information
       *        for the caller as they can attempt to load the file using the old
       *        deserialization methods.
       */
      XmlReaderException(std::string msg, bool binary) :
         XmlBase::XmlException(msg),
         mBin(binary) {}

      virtual ~XmlReaderException() {}

      /**
       * Returns the state of the \e binary flag.
       *
       * @return Is this possibly a binary file?
       */
      bool isBinary()
      {
         return mBin;
      }

      using XmlException::str;
      /**
       * This override appends an additional notice when the
       * file may be binary.
       *
       * @return The message associated with this exception.
       */
      virtual std::string str()
      {
         std::string msg(XmlBase::XmlException::str());
         if (isBinary())
         {
            msg += " :: file may be binary";
         }

         return msg;
      }

   private:
      bool mBin;
   };

   /**
    * An exception class representing an error or warning while parsing the DOM
    */
   class DomParseException : public XmlException
   {
   public:
      /**
       * Create a new %DomParseException.
       *
       * @param msg
       *        The message associated with this exception.
       *
       * @param pNode
       *        The DOM node that caused the exception.
       */
      DomParseException(std::string msg, XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pNode) :
         XmlBase::XmlException(msg),
         mpNode(pNode) {}

      /**
       * Destroy and cleanup this exception.
       */
      virtual ~DomParseException() {}

      /**
       * Find out what DOM node caused the exception.
       *
       * @return The DOMNode that caused the exception.
       */
      XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* where()
      {
         return mpNode;
      }

      using XmlException::str;
      /**
       * Get the message associated with this exception.
       *
       * This will append information about which DOM node
       * caused the exception.
       *
       * @return The message associated with this exception.
       */
      virtual std::string str()
      {
         std::string msg(XmlBase::XmlException::str());
         if (where() != NULL)
         {
            msg += " :: nodename=";
            msg += A(where()->getNodeName());
         }
         return msg;
      }

   private:
      XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* mpNode;
   };

private:
   XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementationLS* mpImpl;
   XERCES_CPP_NAMESPACE_QUALIFIER DOMLSParser* mpParser;
   std::string mXmlSchemaLocation;
   XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* mpDoc;
   XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult* mpResult;
};

/**
 *  This macro creates a for-loop that iterates over the child nodes of a 
 *  specified parent node.
 *
 *  @param pParent
 *        A non-\c NULL node over whose children to iterate
 *  @param pChild
 *        The variable name that will be given to each child node.
 */
#define FOR_EACH_DOMNODE(pParent, pChild) \
   for (XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pChild = pParent->getFirstChild(); \
      pChild != NULL; \
      pChild = pChild->getNextSibling())

/**
 *  This is a convenience method that returns a decendent of a DOMElement in 
 *  an XML hierarchy.
 *
 *  @note This can be inefficient if called multiple times in a method. In this
 *        case, it is usually best to manually loop through the child nodes and
 *        process each as it is traversed.
 *
 *  @param pParent
 *            The DOMElement to start the search from.
 *
 *  @param pName
 *            The full path of the decendent node from the specified parent, with 
 *            element parentage specified with '/'. For example, the following
 *            code will return the node called "baz" from the element
 *            called "bar", which is a child of the element called "foo" which
 *            is a child of pParent.
 *  @code
 *  DOMNode *pNode = findChildNode(pParent, "foo/bar/baz");
 *  @endcode
 *
 *  @return The found node or \c NULL if the node is not found.
 */
XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* findChildNode(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pParent,
   const char* pName);

/**
 *  This is a convenience method that returns an attribute of a DOMElement in 
 *  an XML hierarchy.
 *
 *  @param pParent
 *            The DOMElement to start the search from.
 *
 *  @param pName
 *            The full path of the attribute from the specified parent, with 
 *            element parentage specified with '/'. For example, the following
 *            code will extract the attribute called "baz" from the element
 *            called "bar", which is a child of the element called "foo" which
 *            is a child of pParent.
 *  @code
 *  string s = findAttribute(pParent, "foo/bar/baz");
 *  @endcode
 *
 *  @return The found attribute, or an empty string if the attribute is not
 *             found.
 */
std::string findAttribute(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pParent, const char* pName);

/**
 *  This method reads a font that was written using XMLWriter::addFontElement.
 *
 *  @param pName
 *            The name the font was written out with.
 *
 *  @param pParent
 *            The DOMElement the data was written to.
 *
 *  @param font
 *            A font to read the data into.
 *
 *  @return The DOMElement the data was read from or \c NULL if the element was
 *            not found.
 */
XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* readFontElement(const char* pName,
   XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pParent, Font& font);

/**
 *  This method reads data from an XML data stream into a container.
 *
 *  This method reads data that was written using XML_ADD_CONTAINER. The data
 *  can be read into a fixed-size array, a pre-allocated STL container of the
 *  desired size, or appended onto an existing STL container via push_backs.
 *
 *  @param pParent
 *            The DOMElement the data was written to.
 *
 *  @param pName
 *            The name of each node to write. This will be the name provided to
 *            XML_ADD_CONTAINER.
 *
 *  @param startIter
 *            The location to begin placing data.
 *  @code
 *  vector<int> v(12, 65); // creates a vector<int> with 12 65's in it
 *  XMLWriter writer("MyRootNode");
 *  XML_ADD_CONTAINER(writer, int, v.begin(), v.end());
 *  ...
 *  XmlReader reader(NULL, false);
 *  DOMDocument* pDoc = reader.parse(file);
 *  DOMElement* pRoot = pDoc->getDocumentElement();
 *  int intArray[12];
 *  vector<int> v(12, 0);
 *  list<int> l;
 *  readContainerElements(pRoot, "int", &intArray[0], 12); // reads 12 values into the array
 *  readContainerElements(pRoot, "int", v.begin(), 12); // reads 12 values into the vector
 *  readContainerElements(pRoot, "int", back_inserter(l), 0); // pushes 12 values onto the list
 *  @endcode
 *
 *  @param countLimit
 *             The maximum number of values to read. If this is 0, all of the
 *             values will be read.
 *
 *  @return The number of values read into the container.
 */

/// \endcond

#endif
