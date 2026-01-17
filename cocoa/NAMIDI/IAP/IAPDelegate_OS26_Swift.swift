//
//  IAPDelegate_OS26_Swift.swift
//  NAMIDI
//
//  Created by abechan on 2026/01/11.
//  Copyright © 2026 Noriyoshi Abe. All rights reserved.
//

import Foundation
import StoreKit

enum ErrorCode: Int {
    case ProductNotfound = 1
    case UnexpectedPurchaseResult = 2
}

@available(macOS 26, *)
@objc class IAPDelegate_OS26_Swift: NSObject {
    @objc static func transactionUpdates() async -> IAPTransaction? {
        for await verificationResult in Transaction.updates {
            switch verificationResult {
            case let .verified(transaction):
                if transaction.revocationDate != nil {
                    continue
                }
                await transaction.finish()
                return makeTransaction(productId: transaction.productID, transactionState: .purchased, error: nil)
            case let .unverified(transaction, verificationError):
                return makeTransaction(productId: transaction.productID, transactionState: .failed, error: verificationError as NSError)
            }
        }
        return nil
    }
    
    @objc static func latestEntitlement(for productId: String) async -> IAPEntitlement? {
        if let verificationResult = await Transaction.latest(for: productId) {
            switch verificationResult {
            case let .verified(transaction):
                let entitlement = IAPEntitlement()
                entitlement.productId = transaction.productID
                entitlement.quantity = Int32(transaction.purchasedQuantity)
                return entitlement
            case .unverified(_, _):
                return nil
            }
        } else {
            return nil
        }
    }
    
    @objc static func products(for identifiers: [String]) async -> [IAPProductInfo] {
        do {
            let products = try await Product.products(for: identifiers)
            
            return products.map { (product: Product) in
                let info = IAPProductInfo()
                info.productId = product.id
                info.displayPrice = product.displayPrice
                return info
            }
        } catch {
            NSLog(error.localizedDescription)
            return []
        }
    }
    
    @objc static func purchase(productId: String) async -> IAPTransaction {
        do {
            let products = try await Product.products(for: [productId])
            
            guard let product = products.first else {
                return makeTransaction(productId: productId, transactionState: .failed, error: NSError(domain: "com.nasequencer", code: ErrorCode.ProductNotfound.rawValue))
            }
            
            let purchaseResult = try await product.purchase()
            switch purchaseResult {
            case .pending:
                return makeTransaction(productId: productId, transactionState: .deferred, error: nil)
            case .userCancelled:
                return makeTransaction(productId: productId, transactionState: .canceled, error: nil)
            case let .success(verificationResult):
                switch verificationResult {
                case let .verified(transaction):
                    await transaction.finish()
                    return makeTransaction(productId: productId, transactionState: .purchased, error: nil)
                case let .unverified(_, verificationError):
                    return makeTransaction(productId: productId, transactionState: .failed, error: verificationError as NSError)
                }
            @unknown default:
                return makeTransaction(productId: productId, transactionState: .unknown, error: NSError(domain: "com.nasequencer", code: ErrorCode.UnexpectedPurchaseResult.rawValue))
            }
        } catch {
            NSLog(error.localizedDescription)
            return makeTransaction(productId: productId, transactionState: .failed, error: error as NSError)
        }    }
    
    @objc static func appStoreSync() async {
        do {
            try await AppStore.sync()
        } catch {
            NSLog(error.localizedDescription)
        }
    }
    
    @objc static func canMakePayments() -> Bool {
        return AppStore.canMakePayments
    }
    
    private static func makeTransaction(productId: String, transactionState: IAPTransactionState, error: NSError?) -> IAPTransaction {
        let transaction = IAPTransaction()
        transaction.productId = productId
        transaction.transactionState = transactionState
        transaction.error = error
        return transaction
    }
}
